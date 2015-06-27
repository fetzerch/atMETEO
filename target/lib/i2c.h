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
 * \defgroup libtarget_i2c I2C (TWI) module
 * \ingroup libtarget
 *
 * \brief Wrapper for accessing built-in I2C (TWI) communication interfaces.
 *
 * Avr::I2c is a class implementing a C++ wrapper for accessing the
 * built-in I2C (TWI) communication interfaces.
 */

/*!
 * \file
 * \ingroup libtarget_i2c
 * \copydoc libtarget_i2c
 */

#include <inttypes.h>  // AVR toolchain doesn't offer cinttypes header

#ifndef F_CPU
#error "F_CPU not defined for i2c.h"
#endif

extern "C" {
    #include <external/i2cmaster/i2cmaster.h>
}

namespace Avr
{

/*!
 * \addtogroup libtarget_i2c
 * \{
 */

/*!
 * \brief A C++ wrapper for accessing the built-in I2C (TWI) communication
 *        interfaces.
 *
 * Usage:
 * \code
 * auto i2c = Avr::I2c;
 * i2c.beginTransmission(0xAB);
 * i2c.write(10);
 * i2c.endTransmission(false); // repeated start
 *
 * uint8_t bytesRead = i2c.requestFrom(0xAB, 3);
 * for (int i = 0; i < bytesRead; i++) {
 *     uint8_t byte i2c.read();
 *     // ...
 * }
 * \endcode
 *
 * \note Avr::I2c is currently limited to I2C (TWI) master functionality.
 * \note The implementation is based on i2cmaster
 *       (http://homepage.hispeed.ch/peterfleury/avr-software.html).
 */
class I2c
{
public:
    /*!
     * \brief Returns the I2c::I2c instance.
     *
     * \return The I2c::I2c instance.
     */
    static I2c& instance()
    {
        return s_instance;
    }

    /*!
     * \brief Starts transmission to I2C (TWI) slave device with given
     *        \p address.
     *
     * Bytes are queued for transmission with write() and sent when calling
     * endTransmission().
     *
     * \param address The address of the slave device.
     */
    void beginTransmission(uint8_t address)
    {
        m_bufferIndex = 0;

        i2c_start((address << 1) | I2C_WRITE);
    }

    /*!
     * \brief Queues data for transmission to a slave device.
     *
     * Has to be called in-between beginTransmission() and endTransmission().
     *
     * \param byte The value to be queued for transmission.
     */
    void write(uint8_t byte)
    {
        if (m_bufferIndex == c_bufferSize)
            return;

        m_buffer[m_bufferIndex++] = byte;
    }

    /*!
     * \brief Ends transmission to slave device.
     *
     * Has to be called after beginTransmission() and write().
     *
     * \param stop Send stop message after transmission. If set to `true` a
     *        stop message is sent to the slave device and the bus is released.
     *        If set to `false` no stop message is sent so that the master can
     *        continue the transmission (repeated start).
     */
    void endTransmission(bool stop = true)
    {
        for (uint8_t index = 0; index < m_bufferIndex; ++index) {
            if (!i2c_write(m_buffer[index]))
                return;
        }

        if (stop)
            i2c_stop();
    }

    /*!
     * \brief Request bytes from an I2C (TWI) slave device
     *
     * The bytes can be retrieved using read().
     *
     * \param address The address of the slave device.
     * \param quantity The number of bytes to request from the slave device.
     * \param stop Send stop message after transmission. If set to `true` a
     *        stop message is sent to the slave device and the bus is released.
     *        If set to `false` no stop message is sent so that the master can
     *        continue the transmission (repeated start).
     * \return The number of bytes received from the slave device.
     */
    uint8_t requestFrom(uint8_t address, uint8_t quantity, bool stop = true)
    {
        quantity = Sensors::min(quantity, c_bufferSize);
        if (quantity == 0)
            return 0;

        if (i2c_start((address << 1) | I2C_READ) != 0)
            return 0;

        m_bufferIndex = 0;
        while (--quantity > 0) {
            m_buffer[m_bufferIndex++] = i2c_readAck();
        }
        m_buffer[m_bufferIndex++] = i2c_readNak();
        m_bufferReadIndex = 0;

        if (stop)
            i2c_stop();

        return m_bufferIndex;
    }

    /*!
     * \brief Reads a byte that was received from a slave device after
     *        requesting with requestFrom().
     *
     * \return The next available byte.
     */
    uint8_t read()
    {
        if (m_bufferReadIndex == m_bufferIndex)
            return 0;

        return m_buffer[m_bufferReadIndex++];
    }

private:
    static I2c s_instance;

    static const uint8_t c_bufferSize = 8;
    uint8_t m_buffer[c_bufferSize];
    uint8_t m_bufferIndex = 0;
    uint8_t m_bufferReadIndex = 0;

    I2c()
    {
        i2c_init();
    }
};

I2c I2c::s_instance = I2c();

/*! \} */  // \addtogroup libtarget_i2c

}  // namespace Avr
