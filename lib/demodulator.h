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
 * \defgroup libsensors_demodulator Demodulator
 * \ingroup libsensors
 *
 * \brief Sensors::Demodulator transforms continuous streams with pulse widths
 *        (for example from RF receivers) into bits.
 */

/*!
 * \file
 * \ingroup libsensors_demodulator
 * \copydoc libsensors_demodulator
 */

#include "utils.h"

namespace Sensors
{

/*!
 * \addtogroup libsensors_demodulator
 * \{
 */

/*!
 * \brief Demodulator status returned from DemodulatorBase::addPulseWidth.
 */
enum class DemodulatorStatus : uint8_t {
    /*!
     * The Demodulator data is complete. The decoded bit can be accessed using
     * Demodulator::getData().
     */
    Complete = 0,

    /*!
     * The Demodulator data is incomplete. More pulse widths have to be added
     * using Demodulator::addPulseWidth() before the data can be accessed.
     */
    Incomplete,

    /*!
     * The added pulse width is out of the specified range. This indicates that
     * the received data is erroneous. Typically Demodulator::reset() needs to
     * be called to start a fresh demodulator run.
     */
    OutOfRangeError
};

/*!
 * \brief Configuration parameter for Demodulator that enables Biphase Mark
 *        demodulation.
 *
 * \tparam TShortMin The minimum width of a pulse recognized as a short pulse.
 * \tparam TShortMax The maximum width of a pulse recognized as a short pulse.
 *                   (\p TShortMin <= \p TShortMax)
 * \tparam TLongMin The minimum width of a pulse recognized as a long pulse.
 *                  (\p TShortMax <= \p TLongMin)
 * \tparam TLongMax The maximum width of a pulse recognized as a long pulse.
 *                  (\p TLongMin <= \p TLongMax)
 *
 * \sa https://en.wikipedia.org/wiki/Biphase_mark_code
 */
template <uint16_t TShortMin, uint16_t TShortMax,
          uint16_t TLongMin, uint16_t TLongMax>
struct BiphaseMark
{
    static_assert(TShortMin <= TShortMax,
                  "TShortMax must be less or equal than TShortMax");
    static_assert(TShortMax <= TLongMin,
                  "TShortMax must be less or equal than TLongMin");
    static_assert(TLongMin <= TLongMax,
                  "TLongMin must be less or equal than TLongMax");
};

/*!
 * \brief Demodulator base implementation.
 *
 * \attention Implementation detail. This class must not be used directly,
 *            it only serves as super class for Demodulator.
 */
template <typename Demodulator>
class DemodulatorBase
{
public:
    /*!
     * \brief Adds the \p pulseWidth value to the Demodulator state.
     *
     * \param pulseWidth Pulse width to add.
     * \return Demodulator state after adding the pulse width.
     */
    DemodulatorStatus addPulseWidth(uint16_t pulseWidth)
    {
        return static_cast<Demodulator *>(this)->internalAddPulseWidth(
                    pulseWidth);
    }

    /*!
     * \brief Returns the converted data.
     *
     * \return The converted data.
     */
    bool getData() const
    {
        return m_data;
    }

    /*!
     * \brief Resets the demodulator state.
     *
     * This method typically needs to be called when addPulseWidth() returned
     * the status DemodulatorStatus::OutOfRangeError so that a fresh
     * demodulator run starts.
     */
    void reset()
    {
        m_data = false;
        return static_cast<Demodulator *>(this)->internalReset();
    }

protected:
    DemodulatorBase()
    {
        reset();
    }

    bool m_data;
};

/*!
 * \brief Transforms continuous streams with pulse widths (for example from RF
 *        receivers) into bits.
 *
 * Usage:
 * \code
 * using namespace Sensors;
 * Demodulator<BiphaseMark<100,400,500,800> demodulator;
 * while (...) {
 *     switch (demodulator.addPulseWidth(pulseWidth)) {
 *     case DemodulatorStatus::Incomplete:
 *         break;
 *     case DemodulatorStatus::Complete:
 *         handle_complete_bit(demodulator.getData());
 *         break;
 *     default:
 *         demodulator.reset();
 *         break;
 *     }
 * }
 * \endcode
 *
 * \tparam TDemodulatorType The demodulation algorithm to apply (for example
 *         BiphaseMark).
 *
 * \sa DemodulatorStatus
 *
 * \extends DemodulatorBase
 */
template <typename TDemodulatorType>
class Demodulator;

template <
        uint16_t TShortMin, uint16_t TShortMax,
        uint16_t TLongMin, uint16_t TLongMax>
class Demodulator<BiphaseMark<TShortMin, TShortMax, TLongMin, TLongMax>>
        : public DemodulatorBase<Demodulator<
                        BiphaseMark<TShortMin, TShortMax, TLongMin, TLongMax>>>
{
    friend class DemodulatorBase<Demodulator<BiphaseMark<
                                    TShortMin, TShortMax, TLongMin, TLongMax>>>;

    DemodulatorStatus internalAddPulseWidth(uint16_t pulseWidth)
    {
        auto result = DemodulatorStatus::OutOfRangeError;

        // Long pulse
        if (isLong(pulseWidth)) {
            m_expectShort = false;  // Ignore single short pulse
            this->m_data = true;
            result = DemodulatorStatus::Complete;
        }

        // Short pulse
        else if (isShort(pulseWidth)) {

            // First short
            if (!m_expectShort) {
                m_expectShort = true;
                result = DemodulatorStatus::Incomplete;
            }

            // Second short
            else {
                m_expectShort = false;
                this->m_data = false;
                result = DemodulatorStatus::Complete;
            }
        }

        return result;
    }

    void internalReset()
    {
        m_expectShort = false;
    }

private:
    static bool isLong(uint16_t value)
    {
        return (value >= TLongMin && value < TLongMax);
    }

    static bool isShort(uint16_t value)
    {
        return (value >= TShortMin && value < TShortMax);
    }

    // For BMC, the bit value 0 is represented by two continuous short pulses.
    bool m_expectShort;
};

/*! \} */  // \addtogroup libsensors_demodulator

}  // namespace Sensors
