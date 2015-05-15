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

#pragma once

/*!
 * \defgroup libtarget_dht22 DHT22 Temperature / Humidity sensor
 * \ingroup libtarget
 *
 * \brief Avr::Dht22 decodes data from DHT22 / AM2302 Temperature and Humidity
 *        sensors.
 */

/*!
 * \file
 * \ingroup libtarget_dht22
 * \copydoc libtarget_dht22
 */

#include "lib/utils.h"

#include "pin.h"

namespace Avr
{

/*!
 * \addtogroup libtarget_dht22
 * \{
 */

/*!
 * \brief Decodes data from DHT22 / AM2302 Temperature and humidity sensors
 *
 * \tparam TPin The Avr::InputOutputPin used to connect the sensor.
 * \sa http://akizukidenshi.com/download/ds/aosong/AM2302.pdf
 */
template <typename TPin>
class Dht22
{
public:
    /*!
     * \brief Determines if the last sensor access was valid.
     *
     * \return `true` if the last sensor access was valid and temperature and
     *         humidity can be read, `false` if an error occurred on the last
     *         sensor access.
     */
    bool isValid() const { return m_valid; }

    /*!
     * \brief Retrieves the cached temperature value.
     *
     * The temperature value is updated by read.
     *
     * \return The cached temperature value.
     */
    float temperature() const {
        return m_temperature;
    }

    /*!
     * \brief Retrieves the cached humidity value.
     *
     * The humidity value is updated by read.
     *
     * \return The cached humidity value.
     */
    float humidity() const {
        return m_humidity;
    }

    /*!
     * \brief Reads the temperature and humidity from the sensor and updates
     *        the cached values.
     * \return `true` if the sensor data could be read, `false` if an error
     *         occurred when accessing the sensor.
     */
    bool read()
    {
        uint8_t bits[5] = {};
        m_valid = false;

        // Request sample from sensor by setting the pin to low
        // (T_be: min 0.8 ms, typ 1 ms, max 20 ms), then to high
        // (T_go: min 20 us, typ 30 us, max 200 us) and wait for an
        // acknowledgement from the sensor.
        m_pin.setOutput();
        m_pin.off();      // T_be
        _delay_ms(1);
        m_pin.on();       // T_go
        m_pin.setInput();

        // The sensor will respond with setting the pin to low and then to
        // high (T_rel, T_reh: min 75 us, typ 80 us, max 85 us).
        if (!waitForFallingEdge(c_timeout * 2)) return false; // T_rel
        if (!waitForRisingEdge()) return false; // T_reh

        // Read 40 bit of sensor data. Each bit is started with a low
        // value (T_LOW: min 48 us, typ 50 us, max 55 us). The value of the bit
        // is signaled by a short (T_H0: min 22 us, typ 26 us, max 30 us) or a
        // long high (T_H1: min 68 us, typ 70 us, max 75 us) value.
        if (!waitForFallingEdge()) return false; // T_LOW
        for (uint8_t byteIndex = 0; byteIndex < 5; byteIndex++) {
            uint8_t byte = 0;
            for (uint8_t bitIndex = 0; bitIndex < 8; bitIndex++) {
                if (!waitForRisingEdge()) return false; // T_H0/H1
                _delay_us(30);
                Sensors::bitWrite(byte, 7 - bitIndex, m_pin.isSet());
                if (!waitForFallingEdge()) return false; // T_LOW
            }
            bits[byteIndex] = byte;
        }

        // Reset sensor.
        m_pin.setOutput();
        m_pin.on();

        // Parse sensor data. The High most temperature bit indicates negative
        // temperature.
        m_humidity = Sensors::word(bits[0], bits[1]) / 10.0;
        m_temperature = Sensors::word(bits[2] & 0x7F, bits[3]) / 10.0;
        if (Sensors::bitRead(bits[2], 7)) m_temperature = -m_temperature;

        // Verify checksum.
        m_valid = ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) == bits[4]);
        return m_valid;
    }

private:
    static const uint16_t c_timeout = F_CPU / 40000;

    TPin m_pin;
    bool m_valid = false;
    float m_temperature = 0.0F;
    float m_humidity = 0.0F;

    bool waitForEdge(bool edge, uint16_t timeout = c_timeout) {
        uint16_t cycle = 0;
        while (m_pin.isSet() != edge)
            if (++cycle >= timeout) return false;
        return true;
    }

    bool waitForFallingEdge(uint16_t timeout = c_timeout)
    {
        return waitForEdge(false, timeout);
    }

    bool waitForRisingEdge(uint16_t timeout = c_timeout)
    {
        return waitForEdge(true, timeout);
    }
};

/*! \} */  // \addtogroup libtarget_dht22

}  // namespace Avr
