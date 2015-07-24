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
 * \defgroup libtarget_bmp180 BMP180 Digital pressure sensor
 * \ingroup libtarget
 *
 * \brief Avr::Bmp180 decodes data from Bosch BMP180 Digital pressure sensors.
 */

/*!
 * \file
 * \ingroup libtarget_bmp180
 * \copydoc libtarget_bmp180
 */

#include "lib/utils.h"

#include "i2c.h"

namespace Avr
{

/*!
 * \addtogroup libtarget_bmp180
 * \{
 */

/*!
 * \brief Decodes data from Bosch BMP180 Digital pressure sensors.
 *
 * \sa http://ae-bst.resource.bosch.com/media/products/dokumente/bmp180/BST-BMP180-DS000-12~1.pdf
 * \sa http://wmrx00.sourceforge.net/Arduino/BMP085-Calcs.pdf
 */
class Bmp180
{
public:
    /*!
     * \brief Sensor operation mode for reading the barometric pressure.
     */
    enum class Mode
    {
        /*!
         * Configure sensor to take 8 samples when reading barometric pressure.
         * Highest precision, but slow and with high power consumption.
         */
        UltraHighResolution,

        /*!
         * Configure sensor to take 4 samples when reading barometric pressure.
         */
        HighResolution,

        /*!
         * Configure sensor to take 2 samples when reading barometric pressure.
         */
        Standard,

        /*!
         * Configure sensor to take 1 sample when reading barometric pressure.
         * Fast and little power consumption, but relatively inaccurate.
         */
        UltraLowPower
    };

    /*!
     * \brief Initializes the BMP180 sensor decoder.
     *
     * \param mode The sensor operation mode.
     */
    Bmp180(Mode mode = Mode::Standard)
        : m_mode(mode)
    {
    }

    /*!
     * \brief Determines if the last sensor access was valid.
     *
     * \return `true` if the last sensor access was valid and temperature and
     *         barometric pressure can be read, `false` if an error occurred
     *         on the last sensor access.
     */
    bool isValid() const { return m_valid; }

    /*!
     * \brief Retrieves the cached temperature value.
     *
     * The temperature value is updated by read.
     *
     * \return The cached temperature value.
     */
    float temperature() const
    {
        return m_temperature;
    }

    /*!
     * \brief Retrieves the cached absolute pressure value.
     *
     * The barometric pressure value is updated by read.
     *
     * \return The cached absolute pressure value.
     */
    float pressure() const
    {
        return m_pressure;
    }

    /*!
     * \brief Retrieves the cached pressure value relative to sea level.
     *
     * The barometric pressure value is updated by read.
     *
     * \param altitude The reference altitude.
     * \return The cached pressure value relative to sea level.
     */
    float pressureAtSeaLevel(float altitude) {
        return (static_cast<float>(m_pressure) /
                pow(1.0f - (static_cast<float>(altitude) / 44330.0f), 5.255f));
    }

    /*!
     * \brief Reads the temperature and pressure from the sensor and updates
     *        the cached values.
     * \return `true` if the sensor data could be read, `false` if an error
     *         occurred when accessing the sensor.
     */
    bool read()
    {
        m_valid = false;
        if (readUInt8(c_chipIdRegister) != c_chipId)
            return false;

        if (!m_initialized) {
            readCalibration();
            calculatePolynomials();
            m_initialized = true;
        }

        // Calculate temperature
        float a = m_fc5 * (readTemperature() - m_fc6);
        m_temperature = a + (m_fmc / (a + m_fmd));

        // Calculate pressure
        uint32_t pr = readPressure();
        uint8_t pr0 = pr & 0xFF;
        uint8_t pr1 = (pr >> 8) & 0xFF;
        uint8_t pr2 = (pr >> 16) & 0xFF;
        float pu = (pr2 * 256.00) + pr1 + (pr0 / 256.00);
        float s = m_temperature - 25.0;
        float x = (m_fx2 * pow(s, 2)) + (m_fx1 * s) + m_fx0;
        float y = (m_fy2 * pow(s, 2)) + (m_fy1 * s) + m_fy0;
        float z = (pu - x) / y;
        m_pressure = ((m_fp2 * pow(z, 2)) + (m_fp1 * z) + m_fp0);

        m_valid = true;
        return true;
    }

private:
    static const uint8_t c_address           = 0x77;
    static const uint8_t c_chipId            = 0x55;
    static const uint8_t c_chipIdRegister    = 0xD0;
    static const uint8_t c_versionRegister   = 0xD1;
    static const uint8_t c_softResetRegister = 0xE0;
    static const uint8_t c_ac1Register       = 0xAA;
    static const uint8_t c_ac2Register       = 0xAC;
    static const uint8_t c_ac3Register       = 0xAE;
    static const uint8_t c_ac4Register       = 0xB0;
    static const uint8_t c_ac5Register       = 0xB2;
    static const uint8_t c_ac6Register       = 0xB4;
    static const uint8_t c_b1Register        = 0xB6;
    static const uint8_t c_b2Register        = 0xB8;
    static const uint8_t c_mbRegister        = 0xBA;
    static const uint8_t c_mcRegister        = 0xBC;
    static const uint8_t c_mdRegister        = 0xBE;
    static const uint8_t c_controlRegister   = 0xF4;
    static const uint8_t c_dataRegister      = 0xF6;

    static const uint8_t c_measureTempCmd    = 0x2E;  //  4.5 ms
    static const uint8_t c_measurePressure0  = 0x34;  //  4.5 ms
    static const uint8_t c_measurePressure1  = 0x74;  //  7.5 ms
    static const uint8_t c_measurePressure2  = 0xB4;  // 13.5 ms
    static const uint8_t c_measurePressure3  = 0xF4;  // 25.5 ms

    Mode m_mode;
    bool m_valid = false;
    bool m_initialized = false;
    float m_temperature = 0.0;
    float m_pressure = 0.0;

    // Calibration
    int16_t m_ac1 = 0, m_ac2 = 0, m_ac3 = 0;
    uint16_t m_ac4 = 0, m_ac5 = 0, m_ac6 = 0;
    int16_t m_b1 = 0, m_b2 = 0, m_mb = 0, m_mc = 0, m_md = 0;

    // Polynomials
    float m_fc5 = 0.0, m_fc6 = 0.0, m_fmc = 0.0, m_fmd = 0.0;
    float m_fx0 = 0.0, m_fx1 = 0.0, m_fx2 = 0.0;
    float m_fy0 = 0.0, m_fy1 = 0.0, m_fy2 = 0.0;
    float m_fp0 = 0.0, m_fp1 = 0.0, m_fp2 = 0.0;

    void readCalibration()
    {
        m_ac1 = readUInt16(c_ac1Register);
        m_ac2 = readUInt16(c_ac2Register);
        m_ac3 = readUInt16(c_ac3Register);
        m_ac4 = readUInt16(c_ac4Register);
        m_ac5 = readUInt16(c_ac5Register);
        m_ac6 = readUInt16(c_ac6Register);
        m_b1  = readUInt16(c_b1Register);
        m_b2  = readUInt16(c_b2Register);
        m_mb  = readUInt16(c_mbRegister);
        m_mc  = readUInt16(c_mcRegister);
        m_md  = readUInt16(c_mdRegister);
    }

    void calculatePolynomials()
    {
        float fc3, fc4, fb1;
        fc3 = 160.0 * pow(2, -15) * m_ac3;
        fc4 = pow(10, -3) * pow(2, -15) * m_ac4;
        fb1 = pow(160, 2) * pow(2, -30) * m_b1;

        // For temperature
        m_fc5 = (pow(2, -15) / 160) * m_ac5;
        m_fc6 = m_ac6;
        m_fmc = (pow(2, 11) / pow(160, 2)) * m_mc;
        m_fmd = m_md / 160.0;

        // For pressure
        m_fx0 = m_ac1;
        m_fx1 = 160.0 * pow(2, -13) * m_ac2;
        m_fx2 = pow(160, 2) * pow(2, -25) * m_b2;
        m_fy0 = fc4 * pow(2, 15);
        m_fy1 = fc4 * fc3;
        m_fy2 = fc4 * fb1;
        m_fp0 = (3791.0 - 8.0) / 1600.0;
        m_fp1 = 1.0 - 7357.0 * pow(2, -20);
        m_fp2 = 3038.0 * 100.0 * pow(2, -36);
    }

    uint16_t readTemperature()
    {
        writeUInt8(c_controlRegister, c_measureTempCmd);
        _delay_ms(5);
        return readUInt16(c_dataRegister);
    }

    uint32_t readPressure()
    {
        switch (m_mode) {
        case Mode::UltraLowPower:
            writeUInt8(c_controlRegister, c_measurePressure0);
            _delay_ms(5);
            break;
        case Mode::Standard:
            writeUInt8(c_controlRegister, c_measurePressure1);
            _delay_ms(8);
            break;
        case Mode::HighResolution:
            writeUInt8(c_controlRegister, c_measurePressure2);
            _delay_ms(14);
            break;
        case Mode::UltraHighResolution:
            writeUInt8(c_controlRegister, c_measurePressure3);
            _delay_ms(26);
            break;
        }

        uint16_t msb = readUInt16(c_dataRegister);
        uint8_t lsb = readUInt8(c_dataRegister + 2);
        return (static_cast<uint32_t>(msb) << 8) | lsb;
    }

    void writeUInt8(uint8_t reg, uint8_t value)
    {
        auto i2c = Avr::I2c::instance();
        i2c.beginTransmission(c_address);
        i2c.write(reg);
        i2c.write(value);
        i2c.endTransmission();
    }

    uint8_t readUInt8(uint8_t reg)
    {
        auto i2c = Avr::I2c::instance();
        i2c.beginTransmission(c_address);
        i2c.write(reg);
        i2c.endTransmission();

        uint8_t result = 0;
        if (i2c.requestFrom(c_address, 1) == 1) {
            result = i2c.read();
        }
        return result;
    }

    uint16_t readUInt16(uint8_t reg)
    {
        auto i2c = Avr::I2c::instance();
        i2c.beginTransmission(c_address);
        i2c.write(reg);
        i2c.endTransmission();

        uint16_t result = 0;
        if (i2c.requestFrom(c_address, 2) == 2) {
            uint8_t msb = i2c.read();
            uint8_t lsb = i2c.read();
            result = Sensors::word(msb, lsb);
        }
        return result;
    }
};

/*! \} */  // \addtogroup libtarget_bmp180

}  // namespace Avr
