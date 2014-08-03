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
 * \file
 * \ingroup libsensors_sensor
 *
 * \brief Sensors::HidekiSensor decodes data received from a Hideki RF 433 MHz
 *        sensor.
 */

#include <stdlib.h>  // AVR toolchain doesn't offer cstdlib header
#include <string.h>  // AVR toolchain doesn't offer cstring header

#include "sensor.h"
#include "demodulator.h"
#include "bitdecoder.h"
#include "rfdevice.h"
#include "utils.h"

namespace Sensors
{

/*!
 * \addtogroup libsensors_sensor
 * \{
 */

/*!
 * \brief Decodes data received from a Hideki RF 433 MHz sensor.
 *
 * Hideki sensors are cheap RF 433 MHz sensors for accessing different weather
 * related data such as temperature, humidity, wind direction and speed and
 * others.
 *
 * \note The current implementation is limited to a Thermo/Hygro sensor (TS53).
 */
class HidekiSensor : public Sensor<HidekiSensor>
{
    friend class Sensor<HidekiSensor>;

public:
    /*!
     * \brief Initializes the Hideki sensor decoder.
     */
    HidekiSensor();

    /*!
     * \brief Determines if the current decoder state is valid.
     *
     * \return `true` if the current decoder state is valid, `false` if the
     *         current state contains data that could not be decoded.
     */
    bool isValid() const;

    /*!
     * \brief Determines if the current data is possibly valid (but not
     *        complete).
     * \return `true` if the current decoder state is valid, `false` if the
     *         current state contains data that could not be decoded.
     */
    bool isPossiblyValid() const;

    /*!
     * \brief Returns the sensor id of the current message.
     *
     * The sensor id can be used to distinguish between multiple sensors
     * of the same type. Hideki sensors change their id when replacing the
     * battery or on a manual sensor reset.
     *
     * \return The sensor id of the current message.
     */
    uint8_t sensorId() const;

    /*!
     * \brief Returns the number of the current message.
     *
     * Hideki sensors retransmit a message 3 times.
     *
     * \return The number of the current message (`1`-`3`).
     */
    uint8_t message() const;

    /*!
     * \name Thermo/Hygro
     *
     * Data only available for messages received from Hideki Thermo/Hygro
     * sensors.
     *
     * \{
     */

    /*!
     * \brief Determines if the current message contains a Thermo/Hygro data
     *        set.
     *
     * \return `true` if the current message contains a Thermo/Hygro data set,
     *         `false` otherwise.
     */
    bool isThermoHygro() const;

    /*!
     * \brief Gets the temperature value of the current message.
     *
     * \return The temperature value of the current message.
     */
    int8_t temperature() const;

    /*!
     * \brief Gets the temperature value of the current message as float.
     *
     * \return The temperature value of the current message.
     */
    float temperatureF() const;

    /*!
     * \brief Gets the humidity value of the current message.
     *
     * \return The humidity value of the current message.
     */
    uint8_t humidity() const;

    /*! \} */  // Thermo/Hygro

private:
    SensorStatus internalSetData(uint8_t *data, size_t length);
    SensorStatus internalAddByte(uint8_t byte);
    void internalReset();

    /*!
     * \brief A message transmitted by a Hideki sensor has a constant size of
     *        10 bytes.
     */
    static const size_t c_length = 10;

    /*!
     * \brief Every message transmitted by a Hideki sensor starts with the
     *        pattern `0x9F`.
     */
    static const uint8_t c_header = 0x9F;

    /*!
     * \name Thermo/Hygro
     *
     * Data only available for messages received from Hideki Thermo/Hygro
     * sensors.
     *
     * \{
     */

    /*!
     * \brief Hideki Thermo/Hygro sensors are recognized by their sensor type
     *        `0x1E`.
     */
    static const uint8_t c_thermoHygro = 0x1E;

    /*!
     * \brief A positive temperature value is indicated by the mask `0xC`.
    */
    static const uint8_t c_thermoHygroTempPositive = 0xC;

    /*!
     * \brief A negative temperature value is indicated by the mask `0x4`.
    */
    static const uint8_t c_thermoHygroTempNegative = 0x4;

    /*! \} */  // Thermo/Hygro

    /*!
     * \brief Gets the header value of the current message.
     *
     * \return The header value of the current message.
     */
    uint8_t header() const;

    /*!
     * \brief Gets the package length value of the current message.
     *
     * The package length is used to differentiate different sensor types.
     *
     * \return The package length value of the current message.
     */
    uint8_t packageLength() const;

    /*!
     * \brief Gets the sensor type of the current message.
     *
     * \return The sensor type of the current message.
     */
    uint8_t sensorType() const;

    /*!
     * \brief Calculates the CRC 1 for the current message.
     *
     * \return The CRC 1 calculated for the current message.
     */
    uint8_t crc1() const;

    /*!
     * \brief Calculates the CRC 2 for the current message.
     *
     * \return The CRC 2 calculated for the current message.
     */
    uint8_t crc2() const;

    /*!
     * \brief The current state of the Hideki sensor decoder.
     */
    uint8_t m_data[c_length];

    /*!
     * \brief The number of bytes added to the current state of the Hideki
     *        sensor decoder.
     */
    uint8_t m_byteIndex;
};

/*! \} */  // \addtogroup libsensors_sensor

/*!
 * \addtogroup libsensors_rfdevice
 * \{
 */

/*!
 * \brief Hideki sensor device implemented using a Sensors::HidekiSensor with
 *        the RF 433 MHz reception parameters.
 *
 * A Hideki sensor uses Biphase Mark coding and LSB bit numbering with one even
 * parity bit. A message is 89 bits long, the parity for the last byte is
 * omitted.
 *
 * \tparam TShortMin Minimum length of a short pulse. Has to be set to 183 us.
 * \tparam TShortMax Maximum length of a short pulse. Has to be set to 726 us.
 * \tparam TLongMin Minimum length of a long pulse. Has to be set to 726 us.
 * \tparam TLongMax Maximum length of a long pulse. Has to be set to 1464 us.
 *
 * \note The parameters \p TShortMin, \p TShortMax, \p TLongMin and
 *       \p TLongMax have to be set to the system tick values representing the
 *       time (in us) documented above.
 */
template <uint16_t TShortMin, uint16_t TShortMax,
          uint16_t TLongMin, uint16_t TLongMax>
using HidekiDevice = RfDevice<
    Demodulator<BiphaseMark<TShortMin, TShortMax, TLongMin, TLongMax>>,
    ByteDecoder<EvenParity, LsbBitNumbering>,
    HidekiSensor,
    89>;

/*! \} */  // \addtogroup libsensors_rfdevice

}  // namespace Sensors
