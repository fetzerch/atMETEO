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
 * \defgroup libsensors_sensor Sensor
 * \ingroup libsensors_rf
 *
 * \brief Sensors::Sensor is the base class for sensor specific data decoding.
 */

/*!
 * \file
 * \ingroup libsensors_sensor
 * \copydoc libsensors_sensor
 */

#include <inttypes.h>  // AVR toolchain doesn't offer cinttypes header
#include <stdlib.h>    // AVR toolchain doesn't offer cstdlib header

namespace Sensors
{

/*!
 * \addtogroup libsensors_sensor
 * \{
 */

/*!
 * \brief Sensor status returned from Sensor::setData() and Sensor::addByte().
 */
enum class SensorStatus : uint8_t {
    /*!
     * The Sensor data is complete. The decoded data can be accessed using
     * sensor specific methods.
     */
    Complete = 0,

    /*!
     * The Sensor data is incomplete. More bytes have to be added using
     * Sensor::addByte() before the data can be accessed.
     */
    Incomplete,

    /*!
     * The Sensor data is invalid. Too many bytes have been added. Typically
     * Sensor::reset() needs to be called to start a fresh decoder run.
     */
    TooMuchData,

    /*!
     * The Sensor data is invalid. The parsing failed. Typically
     * Sensor::reset() needs to be called to start a fresh decoder run.
     */
    InvalidData
};

/*!
 * \brief Base class for sensor implementations.
 *
 * Usage:
 * \code
 * using namespace Sensors;
 * MySensor mySensor;
 * Sensor &sensor = mySensor;
 * while (...) {
 *     switch (sensor.addByte(byte)) {
 *     case SensorStatus::Incomplete:
 *         break;
 *     case SensorStatus::Complete:
 *         handle_complete_sensor_data(mySensor.getData());
 *         break;
 *     default:
 *         sensor.reset();
 *         break;
 *     }
 * }
 * \endcode
 *
 * Accessing sensor data is specific to the sensor therefore there is no
 * generic API defined.
 *
 * The Sensor API has to be implemented by specific sensor decoders:
 * \code
 * class MySensor : public Sensor<MySensor> {
 *     friend class Sensor<MySensor>;
 *     // ...
 *     SensorStatus internalSetData(uint8_t *data, size_t length);
 *     SensorStatus internalAddByte(uint8_t byte);
 *     void internalReset();
 *     // ...
 * };
 * \endcode
 *
 * \tparam TSensor The specific sensor decoder.
 *
 * \attention This class must not be used directly, it only serves as super
 *            class for specific sensors.
 */
template <typename TSensor>
class Sensor
{
public:
    /*!
     * \brief Sets the sensor state to the given \p data buffer.
     *
     * \param data Sensor \p data buffer.
     * \param length Length of the \p data buffer.
     * \return Sensor state after setting the \p data buffer.
     */
    SensorStatus setData(uint8_t *data, size_t length)
    {
        return static_cast<TSensor *>(this)->internalSetData(data, length);
    }

    /*!
     * \brief Adds the \p byte to the sensor state.
     *
     * \param byte Byte to add.
     * \return Sensor state after adding the \p byte.
     */
    SensorStatus addByte(uint8_t byte)
    {
        return static_cast<TSensor *>(this)->internalAddByte(byte);
    }

    /*!
     * \brief Resets the state of the sensor decoder for receiving a new data
     *        set.
     */
    void reset()
    {
        return static_cast<TSensor *>(this)->internalReset();
    }
};

/*! \} */  // \addtogroup libsensors_sensor

}  // namespace Sensor
