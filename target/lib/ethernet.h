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
 * \defgroup libtarget_ethernet Ethernet module
 * \ingroup libtarget
 *
 * \brief Utilities for Ethernet communication interfaces.
 *
 * Avr::Ethernet is a class implementing a C++ wrapper for accessing Ethernet
 * communication interfaces.
 */

/*!
 * \file
 * \ingroup libtarget_ethernet
 * \copydoc libtarget_ethernet
 */

#include <inttypes.h>  // AVR toolchain doesn't offer cinttypes header

namespace Avr
{

class Wiznet;

/*!
 * \addtogroup libtarget_ethernet
 * \{
 */

/*!
 * \brief Represents an Ethernet MAC address.
 */
class MacAddress
{
public:
    /*!
     * \brief Initializes a MAC address from octets.
     *
     * \param first_octet The first octet of the MAC address.
     * \param second_octet The second octet of the MAC address.
     * \param third_octet The third octet of the MAC address.
     * \param fourth_octet The fourth octet of the MAC address.
     * \param fifth_octet The fifth octet of the MAC address.
     * \param sixth_octet The sixth octet of the MAC address.
     */
    MacAddress(uint8_t first_octet, uint8_t second_octet,
               uint8_t third_octet, uint8_t fourth_octet,
               uint8_t fifth_octet, uint8_t sixth_octet)
    {
        m_address[0] = first_octet;
        m_address[1] = second_octet;
        m_address[2] = third_octet;
        m_address[3] = fourth_octet;
        m_address[4] = fifth_octet;
        m_address[5] = sixth_octet;
    }

private:
    uint8_t m_address[6];

    friend class Wiznet;
    const uint8_t *rawAddress() const { return m_address; }
};

/*!
 * \brief Represents an IPv4 address
 */
class IpAddress
{
public:
    /*!
     * \brief Initializes an IP address to 0.0.0.0.
     */
    IpAddress()
    {
    }

    /*!
     * \brief Initializes an IP address from octets.
     *
     * \param first_octet The first octet of the IP address.
     * \param second_octet The second octet of the IP address.
     * \param third_octet The third octet of the IP address.
     * \param fourth_octet The fourth octet of the IP address.
     */
    IpAddress(uint8_t first_octet, uint8_t second_octet,
              uint8_t third_octet, uint8_t fourth_octet)
    {
        m_address[0] = first_octet;
        m_address[1] = second_octet;
        m_address[2] = third_octet;
        m_address[3] = fourth_octet;
    }

private:
    uint8_t m_address[4];

    friend class Wiznet;
    const uint8_t *rawAddress() { return m_address; }
};

/*!
 * \brief A C++ wrapper for accessing Ethernet communication interfaces.
 *
 * Usage:
 * \code
 * auto ethernet = Avr::Ethernet<Avr::Wiznet>(
 *                 Avr::MacAddress(0x00, 0x16, 0x36, 0xDE, 0x58, 0xF6),
 *                 Avr::IpAddress(192.168, 0, 200),
 *                 Avr::IpAddress(255, 255, 0, 0));
 * auto dest = Avr::IpAddress(192, 168, 0, 100);
 * ethernet.sendUdpMessage("TEST\n", dest, 8600);
 * \endcode
 *
 * \tparam TDriver The driver for the Ethernet module to use.
 *
 * \note Avr::Ethernet is currently limited to the sending of UDP messages.
 */
template <typename TDriver>
class Ethernet : private TDriver
{
public:
    /*!
     * \brief Initializes the Ethernet driver.
     *
     * After initialization, the Ethernet module replies to ICMP echo requests.
     *
     * \param mac The local Avr::MacAddress. This must be unique withing
     *            the local network.
     * \param ip The local Avr::IpAddress.
     * \param subnet The local subnet configuration.
     */
    Ethernet(MacAddress mac, IpAddress ip, IpAddress subnet)
        : TDriver(mac, ip, subnet)
    {
    }

    /*!
     * \brief Sends an UDP message to the specified destination.
     *
     * \param dest The destination Avr::IpAddress to transmit the message to.
     * \param port The destination UDP port number to transmit the message to.
     * \param message The message to transmit.
     *
     * \return `true` if the message was sent successfully, `false` if the
     *         driver reported an error during transmission. Since UDP is
     *         connectionless, there is no guarantee that the message has been
     *         delivered even if this message reports success.
     */
    bool sendUdpMessage(IpAddress dest, uint16_t port, const char *message)
    {
        return TDriver::internalSendUdpMessage(dest, port, message);
    }
};

/*! \} */  // \addtogroup libtarget_ethernet

}  // namespace Avr
