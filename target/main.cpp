/*
 * atMETEO - An ATmega based weather station
 * Copyright (C) 2014-2015 Christian Fetzer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*!
 * \defgroup application Main application
 * \ingroup target
 *
 * \brief The main weather station application utilizing \ref libsensors
 *        and \ref libtarget.
 *
 * Setup:
 * - Receives \a temperature and \a humidity from a Hideki Thermo/Hygro sensor
 *   connected to the AVR's input capture pin (ICP).
 * - Receives \a temperature and \a humidity from a DHT22 sensor connected to
 *   the AVR's digital I/O pin PD2.
 * - Receives \a temperature and \a pressure from a Bosch BMP180 sensor
 *   connected to the AVR's I2C (TWI) interface.
 * - Receives \a ambient_temperature and \a object_temperature from a Melexis
 *   MLX90614 sensor connected to the AVR's I2C (TWI) interface.
 * - Receives \a sensor_resistance from a Figaro TGS 2600 sensor connected
 *   to the AVR's Analog to Digital Conversion pin 0 (ADC0).
 *
 * The received sensor values are transmitted over the UART interface as
 * JSON object.
 *
 * JSON Schema:
 * \code
 * {
 *     "type": "object",
 *     "properties": {
 *         "rf433": {
 *             "type": "object",
 *             "properties": {
 *                 "temperature": {
 *                     "description":
 *                         "Temperature (in °C) from wireless sensor.",
 *                     "type": "number"
 *                 },
 *                 "humidity": {
 *                     "description": "Humidity (in %) from wireless sensor.",
 *                     "type": "integer",
 *                     "minimum": 0,
 *                     "maximum": 100
 *                 }
 *             }
 *         },
 *         "dht22": {
 *             "type": "object",
 *             "properties": {
 *                 "temperature": {
 *                     "description":
 *                         "Temperature (in °C) from DHT22 sensor.",
 *                     "type": "number"
 *                 },
 *                 "humidity": {
 *                     "description": "Humidity (in %) from DHT22 sensor.",
 *                     "type": "number",
 *                     "minimum": 0,
 *                     "maximum": 100
 *                 }
 *             }
 *         },
 *         "bmp180": {
 *             "type": "object",
 *             "properties": {
 *                 "temperature": {
 *                     "description":
 *                         "Temperature (in °C) from BMP180 sensor.",
 *                     "type": "number"
 *                 },
 *                 "pressure": {
 *                     "description":
 *                         "Barometric pressure (in hPa) from BMP180 sensor.",
 *                     "type": "number"
 *                 },
 *             }
 *         },
 *         "mlx90614": {
 *             "type": "object",
 *             "properties": {
 *                 "ambient_temperature": {
 *                     "description":
 *                         "Ambient temperature (in °C) from MLX90614 sensor.",
 *                     "type": "number"
 *                 },
 *                 "object_temperature": {
 *                     "description":
 *                         "Object temperature (in °C) from MLX90614 sensor.",
 *                     "type": "number",
 *                 }
 *             }
 *         },
 *         "tgs2600": {
 *             "type": "object",
 *             "properties": {
 *                 "sensor_resistance": {
 *                     "description":
 *                         "Sensor resistance in Ohm from TGS 2600 sensor.",
 *                     "type": "integer",
 *                     "minimum": 0
 *                 }
 *             }
 *         }
 *     }
 * }
 * \endcode
 *
 * \sa Sensors::HidekiSensor
 * \sa Avr::Dht22
 * \sa Sensors::Tgs2600
 */

/*!
 * \file
 * \ingroup application
 * \copydoc application
 */

#include <inttypes.h>  // AVR toolchain doesn't offer cinttypes header

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "lib/hidekisensor.h"
#include "lib/dht22.h"
#include "lib/bmp180.h"
#include "lib/mlx90614.h"
#include "lib/tgs2600.h"

#include "lib/adc.h"
#include "lib/uart.h"
#include "lib/timer.h"
#include "lib/pin.h"
#include "lib/atomic.h"

static const uint16_t BAUD = 9600;
static const uint16_t PRESCALER = 8;
static const uint32_t TGS2600_LOADRESISTOR = 10000;
static const uint32_t ALTITUDE = 470;
static const uint32_t DELAY = 30000;

static Sensors::HidekiDevice<
    Avr::TimerUtils<PRESCALER>::usToTicks<183>(),  // Short min
    Avr::TimerUtils<PRESCALER>::usToTicks<726>(),  // Short max
    Avr::TimerUtils<PRESCALER>::usToTicks<726>(),  // Long min
    Avr::TimerUtils<PRESCALER>::usToTicks<1464>()  // Long max
    > s_hidekiDevice;
static Sensors::HidekiData s_hidekiData;

class TimerObserver
{
protected:
    static void pulseWidthReceived(uint16_t pulseWidth)
    {
        if (s_hidekiDevice.addPulseWidth(pulseWidth) ==
                Sensors::RfDeviceStatus::Complete) {
            s_hidekiData.storeSensorValues(s_hidekiDevice);
        }
    }
};

/*!
 * \brief Application entry point.
 *
 * \return As main application, this function does not return.
 */
int main()
{
    Avr::TimerInputCapture<PRESCALER, TimerObserver>::instance();

    sei();  // Enable interrupts

    Avr::Dht22<Avr::InputOutputPin<Avr::DigitalIoD, PD2>> dht22;
    Avr::Bmp180 bmp180(Avr::Bmp180::Mode::UltraHighResolution);
    Avr::Mlx90614 mlx90614;

    Sensors::Tgs2600<TGS2600_LOADRESISTOR> tgs2600;

    // Arduino boards restart when a serial connection is established (DTR).
    // Transmitting an initial string allows the client to know that the
    // microcontroller is ready.
    Avr::Uart<BAUD>::instance().sendLine("READY");

    while (true) {
        auto uart = Avr::Uart<BAUD>::instance();

        // Copy Hideki sensor result set
        Sensors::HidekiData hidekiData;
        {
            Avr::AtomicGuard<Avr::AtomicRestoreState> atomicGuard;
            if (s_hidekiData.isValid()) {
                hidekiData = s_hidekiData;
                s_hidekiData.reset();
            }
        }

        if (hidekiData.isValid()) {
            uart.sendString("{\"rf433\":");
            uart.sendString("{\"temperature\":");
            uart.sendDouble(hidekiData.temperatureF());
            uart.sendString(",\"humidity\":");
            uart.sendUInt(hidekiData.humidity());
            uart.sendString("}}\n");
        }

        if (dht22.read()) {
            uart.sendString("{\"dht22\":");
            uart.sendString("{\"temperature\":");
            uart.sendDouble(dht22.temperature());
            uart.sendString(",\"humidity\":");
            uart.sendDouble(dht22.humidity());
            uart.sendString("}}\n");
        }

        if (bmp180.read()) {
            uart.sendString("{\"bmp180\":");
            uart.sendString("{\"temperature\":");
            uart.sendDouble(bmp180.temperature());
            uart.sendString(",\"pressure\":");
            uart.sendDouble(bmp180.pressureAtSeaLevel(ALTITUDE));
            uart.sendString("}}\n");
        }

        if (mlx90614.read()) {
            uart.sendString("{\"mlx90614\":");
            uart.sendString("{\"ambient_temperature\":");
            uart.sendDouble(mlx90614.ambientTemperature());
            uart.sendString(",\"object_temperature\":");
            uart.sendDouble(mlx90614.objectTemperature());
            uart.sendString("}}\n");
        }

        uart.sendString("{\"tgs2600\":");
        uart.sendString("{\"sensor_resistance\":");
        auto adc = Avr::Adc::instance().readMilliVolts(0, 5);
        uart.sendUInt(tgs2600.sensorResistance(adc));
        uart.sendString("}}\n");

        _delay_ms(DELAY);
    }

    return 0;
}
