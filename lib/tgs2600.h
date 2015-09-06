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
 * \defgroup libsensors_tgs2600 Figaro TGS 2600
 * \ingroup libsensors
 *
 * \brief Sensors::Tgs2600 decodes data from Figaro TGS 2600 air contaminants
 *        sensors.
 */

/*!
 * \file
 * \ingroup libsensors_tgs2600
 * \copydoc libsensors_tgs2600
 */

#include <inttypes.h>  // AVR toolchain doesn't offer cinttypes header

namespace Sensors
{

/*!
 * \addtogroup libsensors_tgs2600
 * \{
 */

/*!
 * \brief Decodes data from Figaro TGS 2600 air contaminants sensors.
 *
 * \tparam TLoadResistance Value of the load resistance Rl as specified in the
 *         datasheet.
 * \sa http://www.figarosensor.com/products/2600pdf.pdf
 */
template <uint32_t TLoadResistance>
class Tgs2600
{
    static_assert(TLoadResistance >= 450, "Minimum load resistance 450 Ohm.");

public:
    /*!
     * \brief Initializes the TGS 2600 sensor decoder.
     */
    Tgs2600()
        : m_referenceResistance(1.0F)
        , m_referenceHumidity(65)
        , m_referenceTemperature(20)
        , m_calibration(1.0F)
    {
    }

    template <class T>
    class unimplemented;

    /*!
     * \brief Retrieves the raw sensor resistance for a given \p vout.
     * \param vout Sensor voltage meassured using an Analog to Digital
     *        Convertor (ADC).
     * \return The sensor resistance or `UINT32_MAX` if \p vout is 0.
     */
    uint32_t sensorResistance(uint16_t vout) const
    {
        static_assert(TLoadResistance <= UINT32_MAX / c_vcc,
                      "Load resistance out of range.");

        if (vout == 0) return UINT32_MAX;
        if (vout >= c_vcc) return 0;
        return ((c_vcc * TLoadResistance) / vout) - TLoadResistance;
    }

    /*!
     * \brief Retrieves the calibrated sensor resistance for a given \p vout.
     *
     * The calibration is based on reference humidity and temperature specified
     * with setReferenceHumidity() and setReferenceTemperature(). See
     * http://www.atmos-meas-tech.net/5/1925/2012/amt-5-1925-2012.pdf for
     * a detailed explanation of the approximation.
     *
     * \param vout Voltage meassured using an Analog to Digital Convertor (ADC).
     * \return The sensor resistance or `UINT32_MAX` if \p vout is 0.
     */
    uint32_t sensorResistanceCalibrated(uint32_t vout) const
    {
        uint32_t sensorResistance = this->sensorResistance(vout);

        if (sensorResistance == UINT32_MAX)
            return UINT32_MAX;

        return m_calibration * sensorResistance;
    }

    /*!
     * \brief Retrieves the calibrated relative sensor resistance for a given
     *        \p vout.
     *
     * The sensor resistance is calculated based on a reference resistance that
     * can be specified with setReferenceResistance().
     *
     * \param vout Voltage meassured using an Analog to Digital Convertor (ADC).
     * \return The sensor resistance.
     * \sa sensorResistanceCalibrated
     */
    float sensorResistanceRelative(uint32_t vout) const
    {
        return sensorResistanceCalibrated(vout) / m_referenceResistance;
    }

    /*!
     * \brief Sets reference \p humitidty for sensorResistanceCalibrated() and
     *        sensorResistanceRelative()
     * \param humidity Reference humidity.
     */
    void setReferenceHumidity(float humidity)
    {
        m_referenceHumidity = humidity;
        updateCalibration();
    }

    /*!
     * \brief Sets reference \p temperature for sensorResistanceCalibrated()
     *        and sensorResistanceRelative().
     * \param temperature Reference temperature.
     */
    void setReferenceTemperature(float temperature)
    {
        m_referenceTemperature = temperature;
        updateCalibration();
    }

    /*!
     * \brief Sets reference \p resistance for sensorResistanceRelative().
     * \param resistance Reference resistance.
     */
    void setReferenceResistance(float resistance)
    {
        m_referenceResistance = resistance;
    }

    /*!
     * \brief Returns the calculated calibration factor influenced by
     *        reference humidity and reference temperature.
     * \return The calculated calibration factor.
     * \sa sensorResistanceCalibrated
     * \sa setReferenceHumidity, setReferenceTemperature
     */
    float calibration() const
    {
        return m_calibration;
    }

private:
    void updateCalibration()
    {
        m_calibration = 0.024F +
                        0.0072F * m_referenceHumidity +
                        0.0246F * m_referenceTemperature;
    }

    // Sensor requires Vc of 5V.
    static const uint16_t c_vcc = 5000;

    float m_referenceResistance;
    float m_referenceHumidity;
    float m_referenceTemperature;
    float m_calibration;
};

/*! \} */  // \addtogroup libsensors_tgs2600

}  // namespace Sensors
