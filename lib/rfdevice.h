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
 * \defgroup libsensors_rfdevice RF Device
 * \ingroup libsensors
 *
 * \brief Sensors::RfDevice is the base template for RF receivers connecting
 *        \ref libsensors_demodulator, \ref libsensors_bitdecoder and
 *        \ref libsensors_sensor.
 */

/*!
 * \file
 * \ingroup libsensors_rfdevice
 * \copydoc libsensors_rfdevice
 */

#include "demodulator.h"
#include "bitdecoder.h"
#include "sensor.h"

namespace Sensors
{

/*!
 * \addtogroup libsensors_rfdevice
 * \{
 */

/*!
 * \brief RfDevice status returned from RfDevice::addPulseWidth().
 */
enum class RfDeviceStatus : uint8_t {
    /*!
     * The RfDevice data is complete. The decoded data can be accessed using
     * sensor specific methods.
     */
    Complete = 0,

    /*!
     * The RfDevice data is incomplete. More pulse widths have to be added
     * using RfDevice::addPulseWidth() before the data can be accessed.
     */
    Incomplete,

    /*!
     * The received data is invalid. A new decoder run is started automatically
     * when new pulse widths are added using RfDevice::addPulseWidth().
     */
    InvalidData
};

/*!
 * \brief Connects \ref libsensors_demodulator, \ref libsensors_bitdecoder and
 *        \ref libsensors_sensor for decoding sensor data from RF receivers.
 *
 * Usage:
 * \code
 * using namespace Sensors;
 * MyDevice myRfDevice;
 * RfDevice &rfdevice = myRfDevice;
 * while (...) {
 *     switch (rfdevice.addPulseWidth(pulseWidth)) {
 *     case SensorStatus::Incomplete:
 *         break;
 *     case RfDeviceStatus::Complete:
 *         handle_complete_rfdevice_data(myRfDevice.getData());
 *         break;
 *     default:
 *         break;
 *     }
 * }
 * \endcode
 *
 * Accessing sensor data is specific to the sensor therefore there is no
 * generic API defined.
 *
 * \tparam TDemodulator The demodulator configuration.
 * \tparam TBitDecoder The demodulator configuration.
 * \tparam TSensor The sensor configuration.
 * \tparam TBitLength Maximum length of a message in bits. If set to a non-zero
 *         value, the RfDevice will expect to receive as many bits as specified
 *         before the \p TSensor is called for decoding the data.
 *
 *  \attention This class must not be used directly, it only serves as template
 *             for specific RF devices.
 */
template <typename TDemodulator,
          typename TBitDecoder,
          typename TSensor,
          uint16_t TBitLength = 0>
class RfDevice :
        private TDemodulator,
        private TBitDecoder,
        public TSensor
{
public:
    /*!
     * \brief Adds the \p pulseWidth value to the RfDevice state.
     *
     * This function will call TDeviceObserver::dataAvailable() as soon
     * as a complete sensor data set has been decoded.
     *
     * \param pulseWidth Pulse width to add.
     * \return RfDevice state after adding the pulse width.
     */
    RfDeviceStatus addPulseWidth(uint16_t pulseWidth)
    {
        if (m_lastStatus == RfDeviceStatus::InvalidData)
            reset();

        m_lastStatus = internalAddPulseWidth(pulseWidth);
        return m_lastStatus;
    }

    /*!
     * \brief Resets the device state.
     */
    void reset()
    {
        TDemodulator::reset();
        TBitDecoder::reset();
        TSensor::reset();
        m_bitLength = 0;
    }

private:
    RfDeviceStatus internalAddPulseWidth(uint16_t pulseWidth)
    {
        auto demodulatorStatus = TDemodulator::addPulseWidth(pulseWidth);
        if (demodulatorStatus == DemodulatorStatus::Incomplete) {
            return RfDeviceStatus::Incomplete;
        } else if (demodulatorStatus != DemodulatorStatus::Complete) {
            return RfDeviceStatus::InvalidData;
        }

        // Bit
        auto decoderStatus = TBitDecoder::addBit(TDemodulator::getData());
        if (decoderStatus == BitDecoderStatus::ParityError) {
            return RfDeviceStatus::InvalidData;
        }
        ++m_bitLength;
        if (m_bitLength != TBitLength &&
            decoderStatus != BitDecoderStatus::Complete) {
            return RfDeviceStatus::Incomplete;
        }

        // Byte
        auto sensorStatus = TSensor::addByte(TBitDecoder::getData());
        if (sensorStatus == SensorStatus::Incomplete) {
            return RfDeviceStatus::Incomplete;
        } else if (sensorStatus != SensorStatus::Complete) {
            return RfDeviceStatus::InvalidData;
        }

        return RfDeviceStatus::Complete;
    }

    uint16_t m_bitLength = 0;
    RfDeviceStatus m_lastStatus = RfDeviceStatus::Incomplete;
};

/*! \} */  // \addtogroup libsensors_rfdevice

}  // namespace Sensors
