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
 * \defgroup libtarget_uart UART module
 * \ingroup libtarget
 *
 * \brief Wrapper for accessing built-in UART communication interfaces.
 *
 * Avr::Uart is a template class implementing a C++ wrapper for accessing the
 * built-in UART communication interfaces.
 */

/*!
 * \file
 * \ingroup libtarget_uart
 * \copydoc libtarget_uart
 */

#include <stdlib.h>

extern "C" {
    #include <external/uart/uart.h>
}

namespace Avr
{

/*!
 * \addtogroup libtarget_uart
 * \{
 */

/*!
 * \brief A C++ wrapper for accessing the built-in UART communication
 *        interfaces.
 *
 * Usage:
 * \code
 * auto uart = Avr::Uart<9600>::instance();
 * uart.sendString("Hello UART!");
 * \endcode
 *
 * This template must not be instantiated more than once.
 *
 * \tparam baud The baud rate used for the transmission.
 *
 * \note Avr::Uart is currently limited to data transmission.
 * \note The implementation is based on avr-uart
 *       (https://github.com/andygock/avr-uart).
 * \warning Sending data has to be used with care in program parts where
 *          interrupts are disabled. The data is added to a send buffer and
 *          transmitted only when interrupts are enabled. This leads to
 *          deadlock when the send buffer becomes full.
 */
template <uint16_t baud>
class Uart
{
public:
    /*!
     * \brief Returns the Avr::Uart instance.
     *
     * \return The Avr::Uart instance.
     */
    static Uart& instance()
    {
        return s_instance;
    }

    /*!
     * \brief Transmits the character \p c.
     *
     * \param c The character to be sent.
     */
    void sendChar(unsigned char c)
    {
        uart0_putc(c);
    }

    /*!
     * \brief Transmits the null-terminated string \p str.
     *
     * \param str The null-terminated string to be sent.
     */
    void sendString(const char *str)
    {
        while (*str) {
            sendChar(*str);
            ++str;
        }
    }

    /*!
     * \brief Transmits the null-terminated string \p str followed by a new
     *        line character.
     *
     * \param str The line to be sent.
     */
    void sendLine(const char *str)
    {
        sendString(str);
        sendChar('\n');
    }

    /*!
     * \brief Transmits the string representation of the unsigned integer
     *        \p value.
     *
     * \param value The unsigned integer value to be sent.
     */
    void sendUInt(uint32_t value)
    {
        char buffer[sizeof(uint32_t) * 8 + 1];
        ultoa(value, buffer, 10);
        sendString(buffer);
    }

    /*!
     * \brief Transmits the string representation of the unsigned integer
     *        \p value and its \p description.
     *
     * The transmission format is `<description>: <value>`.
     *
     * \param description A null-terminated string describing the \p value.
     * \param value The unsigned integer value to be sent.
     *
     * \sa sendUInt
     */
    void sendValue(const char *description, uint32_t value)
    {
        sendString(description);
        sendString(": ");
        sendUInt(value);
        sendChar('\n');
    }

    /*!
     * \brief Transmits the string representation of the double \p value.
     *
     * The transmission format is `[sign]ddd.dd`.
     *
     * \param value The double value to be sent.
     */
    void sendDouble(double value)
    {
        if (value > 999 || value < -999) value = -1;
        char buffer[7];
        dtostrf(value, 4, 2, buffer);
        sendString(buffer);
    }

private:
    static Uart s_instance;

    Uart()
    {
        uart0_init(UART_BAUD_SELECT(baud, F_CPU));
    }
};

template <uint16_t baud>
Uart<baud> Uart<baud>::s_instance = Uart();

/*! \} */  // \addtogroup libtarget_uart

}  // namespace Avr
