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

#include <inttypes.h>  // AVR toolchain doesn't offer cinttypes header

#include "lib/utils.h"

#include "pin.h"
#include "spi.h"
#include "ethernet.h"

namespace Avr
{

/*!
 * \brief Avr::Ethernet driver for WIZnet Ethernet modules (W5100, W5200,
 *        W5300, W5500).
 *
 * The usage is described in the documentation of the Avr::Ethernet module.
 *
 * \note The implementation is based on the WIZnet ioLibrary
 *       (https://github.com/Wiznet/ioLibrary_Driver).
 */
class Wiznet
{
protected:
    Wiznet(MacAddress mac, IpAddress ip, IpAddress subnet);

    bool internalSendUdpMessage(IpAddress dest, uint16_t port,
                                const char *message);

private:
    static void chipselect()
    {
        Spi<Avr::DigitalIoB, PB2>::instance().select();
    }

    static void chipdeselect()
    {
        Spi<Avr::DigitalIoB, PB2>::instance().deselect();
    }

    static uint8_t read()
    {
        return Spi<Avr::DigitalIoB, PB2>::instance().receive();
    }

    static void write(uint8_t byte)
    {
        Spi<Avr::DigitalIoB, PB2>::instance().transmit(byte);
    }
};

}  // namespace Avr
