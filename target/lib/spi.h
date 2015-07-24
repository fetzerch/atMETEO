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
 * \defgroup libtarget_spi SPI module
 * \ingroup libtarget
 *
 * \brief Wrapper for accessing built-in SPI communication interface.
 *
 * Avr::Spi is a class implementing a C++ wrapper for accessing the
 * built-in SPI communication interface.
 */

/*!
 * \file
 * \ingroup libtarget_spi
 * \copydoc libtarget_spi
 */

#include <inttypes.h>  // AVR toolchain doesn't offer cinttypes header

#include "lib/utils.h"

namespace Avr
{

/*!
 * \addtogroup libtarget_spi
 * \{
 */

/*!
 * \brief A C++ wrapper for accessing the built-in SPI communication
 *        interface.
 *
 * Usage:
 * \code
 * auto spi = Avr::Spi<DigitalIoB, PB2>::instance();
 * spi.select();
 * uint8_t value = spi.transceive(0x10);
 * spi.deselect();
 * \endcode
 *
 * \tparam TCsDigitalIo The Avr::DigitalIo configuration for the chip select
 *         pin.
 * \tparam csPinNumber The number of the chip select pin.
 *
 * \note Avr::Spi is currently limited to SPI master functionality.
 */
template <class TCsDigitalIo, uint8_t csPinNumber>
class Spi
{
public:
    /*!
     * \brief Returns the Avr::Spi instance.
     *
     * \return The Avr::Spi instance.
     */
    static Spi& instance()
    {
        return s_instance;
    }

    /*!
     * \brief Signals chip select to slave.
     */
    void select()
    {
        s_cs.on();
    }

    /*!
     * \brief Signals chip deselect to slave.
     */
    void deselect()
    {
        s_cs.off();
    }

    /*!
     * \brief Sends the byte \p value and receives a byte over SPI.
     * \param value Byte to transmit.
     * \return The received byte.
     */
    uint8_t transceive(uint8_t value)
    {
        SPDR = value;
        while (!(Sensors::bitRead(SPSR, SPIF)));
        return SPDR;
    }

    /*!
     * \brief Sends the byte \p value over SPI.
     * \param value Byte to transmit.
     */
    void transmit(uint8_t value)
    {
        transceive(value);
    }

    /*!
     * \brief Receives a byte over SPI.
     * \return The received byte.
     */
    uint8_t receive()
    {
        return transceive(0x00);
    }

private:
    static Spi s_instance;
    Avr::OutputPinInverted<TCsDigitalIo, csPinNumber> s_cs;

    Spi()
    {
        Avr::OutputPin<Avr::DigitalIoB, PB5> clk;
        Avr::OutputPin<Avr::DigitalIoB, PB3> mosi;
        Avr::InputPin<Avr::DigitalIoB, PB4> miso;

        deselect();
        Sensors::bitSet(SPCR, SPE);   // Enable SPI
        Sensors::bitSet(SPCR, MSTR);  // SPI Master
        Sensors::bitSet(SPSR, SPI2X); // Clock rate
    }
};

template <class TCsDigitalIo, uint8_t csPinNumber>
Avr::Spi<TCsDigitalIo, csPinNumber>
Spi<TCsDigitalIo, csPinNumber>::s_instance = Spi();

/*! \} */  // \addtogroup libtarget_spi

}  // namespace Avr
