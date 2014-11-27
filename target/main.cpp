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
 *
 * The received sensor values are transmitted over the UART interface as
 * JSON object.
 *
 * JSON Schema:
 * \code
 * {
 *     "type": "object",
 *     "properties": {
 *         "temperature": {
 *             "description": "Temperature (in °C) from wireless sensor.",
 *             "type": "number"
 *         },
 *         "humidity": {
 *             "description": "Humindity (in %) from wireless sensor.",
 *             "type": "integer",
 *             "minimum": 0,
 *             "maximum": 100
 *     }
 * }
 * \endcode
 *
 * \sa Sensors::HidekiSensor
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

#include "lib/uart.h"
#include "lib/timer.h"

static const uint16_t BAUD = 9600;
static const uint16_t PRESCALER = 8;

static Sensors::HidekiDevice<
    Avr::TimerUtils<PRESCALER>::usToTicks<183>(),  // Short min
    Avr::TimerUtils<PRESCALER>::usToTicks<726>(),  // Short max
    Avr::TimerUtils<PRESCALER>::usToTicks<726>(),  // Long min
    Avr::TimerUtils<PRESCALER>::usToTicks<1464>()  // Long max
    > s_device;

class TimerObserver
{
protected:
    static void pulseWidthReceived(uint16_t pulseWidth)
    {
        if (s_device.addPulseWidth(pulseWidth) ==
                Sensors::RfDeviceStatus::Complete) {
            auto uart = Avr::Uart<BAUD>::instance();
            uart.sendString("{\"temperature\":");
            uart.sendDouble(s_device.temperatureF());
            uart.sendString(",\"humidity\":");
            uart.sendUInt(s_device.humidity());
            uart.sendString("}\n");
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

    // Arduino boards restart when a serial connection is established (DTR).
    // Transmitting an initial string allows the client to know that the
    // microcontroller is ready.
    Avr::Uart<BAUD>::instance().sendLine("READY");

    while (true);

    return 0;
}
