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
 * \defgroup libtarget_mlx90614 MLX90614 Infrared thermometer
 * \ingroup libtarget
 *
 * \brief Avr::Mlx90614 decodes data from Melexis MLX90614 Infrared thermometer
 *        sensors.
 */

/*!
 * \file
 * \ingroup libtarget_mlx90614
 * \copydoc libtarget_mlx90614
 */

#include "lib/utils.h"

#include "lib/i2c.h"

namespace Avr
{

/*!
 * \addtogroup libtarget_mlx90614
 * \{
 */

/*!
 * \brief Decodes data from Melexis MLX90614 Infrared thermometer sensors.
 *
 * \sa http://www.melexis.com/Asset/IR-sensor-thermometer-MLX90614-Datasheet-DownloadLink-5152.aspx
 */
class Mlx90614
{
public:
    /*!
     * \brief Determines if the last sensor access was valid.
     *
     * \return `true` if the last sensor access was valid and temperature can
     *         be read, `false` if an error occurred on the last sensor access.
     */
    bool isValid() const { return m_valid; }

    /*!
     * \brief Retrieves the cached ambient temperature value.
     *
     * The ambient temperature value is updated by read.
     *
     * \return The cached ambient temperature value.
     */
    float ambientTemperature() const {
        return m_ambientTemperature;
    }

    /*!
     * \brief Retrieves the cached object temperature value.
     *
     * The object temperature value is updated by read.
     *
     * \return The cached object temperature value.
     */
    float objectTemperature() const {
        return m_objectTemperature;
    }

    /*!
     * \brief Reads the ambient and object temperature from the sensor and
     *        updates the cached values.
     * \return `true` if the sensor data could be read, `false` if an error
     *         occurred when accessing the sensor.
     */
    bool read()
    {
        bool status;
        m_valid = true;

        m_ambientTemperature = read(c_ambientTemperatureAddress, &status)
                * c_resolution - c_zeroCinK;
        if (!status)
            m_valid = false;

        m_objectTemperature = read(c_objectTemperatureAddress, &status)
                * c_resolution - c_zeroCinK;
        if (!status)
            m_valid = false;

        return m_valid;
    }

private:
    static constexpr float c_resolution = 0.02F;
    static constexpr float c_zeroCinK = 273.15F;
    static const uint8_t c_deviceAddress = 0x5A;
    static const uint8_t c_ambientTemperatureAddress = 0x06;
    static const uint8_t c_objectTemperatureAddress = 0x07;

    bool m_valid = false;
    float m_ambientTemperature = 0.0F;
    float m_objectTemperature = 0.0F;

    uint16_t read(uint8_t reg, bool *status)
    {
        auto i2c = Avr::I2c::instance();
        i2c.beginTransmission(c_deviceAddress);
        i2c.write(reg);
        i2c.endTransmission(false);

        uint8_t bytesRead = i2c.requestFrom(c_deviceAddress, 3);
        uint16_t value = 0;
        if (bytesRead == 3) {
            uint8_t lsb = i2c.read();
            uint8_t msb = i2c.read();

            // Bit 7 set indicates error
            if (!Sensors::bitRead(msb, 7)) {
                value = Sensors::word(msb & 0x7F, lsb);
                if (status) *status = true;
            } else {
                if (status) *status = false;
            }
        } else {
            if (status) *status = false;
        }
        return value;
    }
};

/*! \} */  // \addtogroup libtarget_mlx90614

}  // namespace Avr
