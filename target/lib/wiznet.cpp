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

#include "wiznet.h"

#include <string.h>

extern "C" {
    #include <external/wiznet/Ethernet/socket.h>
}

namespace Avr
{

bool Wiznet::internalSendUdpMessage(
        IpAddress dest, uint16_t port, const char *message)
{
    int8_t so = socket(0, Sn_MR_UDP, 0, 0);
    if (so < 0)
        return false;

    int32_t status = sendto(so, reinterpret_cast<unsigned char*>(
                            const_cast<char *>(message)), strlen(message),
                            const_cast<uint8_t *>(dest.rawAddress()), port);
    close(so);
    return status > 0;
}

Wiznet::Wiznet(
        MacAddress mac, IpAddress ip, IpAddress subnet)
{
    chipdeselect();

    reg_wizchip_cs_cbfunc(chipselect, chipdeselect);
    reg_wizchip_spi_cbfunc(read, write);

    uint8_t memsize[2][4] = { { 2, 2, 2, 2 }, { 2, 2, 2, 2 } };
    ctlwizchip(CW_INIT_WIZCHIP, &memsize);

    auto config = wiz_NetInfo();
    memcpy(config.mac, mac.rawAddress(), 6);
    memcpy(config.ip, ip.rawAddress(), 4);
    memcpy(config.sn, subnet.rawAddress(), 4);
    config.dhcp = NETINFO_STATIC;
    ctlnetwork(CN_SET_NETINFO, &config);
}

}  // namespace Avr
