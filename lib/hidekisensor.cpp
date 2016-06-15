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

#include "hidekisensor.h"

namespace Sensors
{

HidekiSensor::HidekiSensor()
{
    reset();
}

SensorStatus HidekiSensor::internalSetData(uint8_t *data, size_t length)
{
    reset();

    if (length > c_length) {
        return SensorStatus::TooMuchData;
    }

    memcpy(m_data, data, length);
    m_byteIndex = length;

    if (!isPossiblyValid()) {
        return SensorStatus::InvalidData;
    }

    if (m_byteIndex == packageLength() + 3) {
        return SensorStatus::Complete;
    } else {
        return SensorStatus::Incomplete;
    }
}

SensorStatus HidekiSensor::internalAddByte(uint8_t byte)
{
    if (m_byteIndex >= c_length) {
        return SensorStatus::TooMuchData;
    }

    m_data[m_byteIndex++] = byte;

    if (!isPossiblyValid()) {
        return SensorStatus::InvalidData;
    }

    if (m_byteIndex == packageLength() + 3) {
        return SensorStatus::Complete;
    } else {
        return SensorStatus::Incomplete;
    }
}

void HidekiSensor::internalReset()
{
    m_byteIndex = 0;
    memset(m_data, 0, sizeof(m_data));
}

bool HidekiSensor::isValid() const
{
    return ((m_byteIndex == packageLength() + 3) &&
            (header() == c_header) &&
            (channel() != 0) &&
            (crc1() == m_data[packageLength() + 1]) &&
            (crc2() == m_data[packageLength() + 2]));
}

bool HidekiSensor::isPossiblyValid() const
{
    if (m_byteIndex > 0 && header() != c_header) {
        return false;
    }

    if (m_byteIndex > 2) {
        if ((m_byteIndex > packageLength() + 1) &&
            (crc1() != m_data[packageLength() + 1])) {
            return false;
        }
        if ((m_byteIndex > packageLength() + 2) &&
            (crc2() != m_data[packageLength() + 2])) {
            return false;
        }
    }

    return true;
}

uint8_t HidekiSensor::channel() const
{
    switch (m_data[1] >> 5) {
    case 1: return 1;
    case 2: return 2;
    case 3: return 3;
    case 4: return 6;
    case 5: return 4;
    case 6: return 5;
    default: return 0;
    }
}

uint8_t HidekiSensor::sensorId() const
{
    return m_data[1] & 0x1F;
}

uint8_t HidekiSensor::message() const
{
    return (m_data[3] >> 6);
}

uint8_t HidekiSensor::header() const
{
    return m_data[0];
}

uint8_t HidekiSensor::packageLength() const
{
    return (m_data[2] >> 1) & 0x1F;
}

uint8_t HidekiSensor::sensorType() const
{
    return (m_data[3] & 0x1F);
}

uint8_t HidekiSensor::crc1() const
{
    uint8_t crc1 = 0;
    for (uint8_t byte = 1; byte < packageLength() + 1; ++byte) {
        crc1 ^= m_data[byte];
    }
    return crc1;
}

/*!
 * \note CRC algorithm has been reverse engineered using CRC RegEng
 * (http://reveng.sourceforge.net/):
 * width=8  poly=0x07  init=0x00  refin=true  refout=true \
 * xorout=0x00  check=0x20  name=(none)
 *
 * This is a CRC-8 LSB algorithm with the polynom 0x07.
 */
uint8_t HidekiSensor::crc2() const
{
    uint8_t crc2 = 0;
    for (uint8_t byte = 1; byte < packageLength() + 2; ++byte) {
        crc2 = crc2 ^ m_data[byte];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            if ((crc2 & 0x01) != 0) {
                crc2 = (crc2 >> 1) ^ 0xE0;
            } else {
                crc2 >>= 1;
            }
        }
    }
    return crc2;
}

// Thermo/Hygro sensor specific functions
bool HidekiSensor::isThermoHygro() const
{
    return (sensorType() == c_thermoHygro && packageLength() == 7);
}

int8_t HidekiSensor::temperature() const
{
    int8_t temp = isThermoHygro() ?
                lowNibble(m_data[5]) * 10 + highNibble(m_data[4]) : 0;
    if (highNibble(m_data[5]) == c_thermoHygroTempNegative)
        temp = -temp;
    return temp;
}

float HidekiSensor::temperatureF() const
{
    float decimal = lowNibble(m_data[4]) * 0.1;
    if (highNibble(m_data[5]) == c_thermoHygroTempNegative)
        decimal = -decimal;

    return isThermoHygro() ? temperature() + decimal : 0;
}

uint8_t HidekiSensor::humidity() const
{
    return isThermoHygro() ?
                highNibble(m_data[6]) * 10 + lowNibble(m_data[6]) : 0;
}

}  // namespace Sensors
