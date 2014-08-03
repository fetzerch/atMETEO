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
 * \defgroup libsensors_bitdecoder Bit Decoder
 * \ingroup libsensors
 *
 * \brief Sensors::BitDecoder transforms continuous bit streams (for example
 *        from RF demodulation) into bytes.
 */

/*!
 * \file
 * \ingroup libsensors_bitdecoder
 * \copydoc libsensors_bitdecoder
 */

#include <limits.h>  // AVR toolchain doesn't offer climits header

#include "utils.h"

namespace Sensors
{

/*!
 * \addtogroup libsensors_bitdecoder
 * \{
 */

/*!
 * \brief BitDecoder status returned from BitDecoder::addBit().
 */
enum class BitDecoderStatus : uint8_t {
    /*!
     * The BitDecoder data is complete. The decoded data can be accessed using
     * BitDecoder::getData().
     */
    Complete = 0,

    /*!
     * The BitDecoder data is incomplete. More bits have to be added using
     * BitDecoder::addBit() before the data can be accessed.
     */
    Incomplete,

    /*!
     * The parity bit in the BitDecoder data is incorrect. Typically
     * BitDecoder::reset() needs to be called to start a fresh decoder run.
     */
    ParityError
};

/*!
 * \brief Configuration parameter for BitDecoder that leads to new bits being
 *        added with MSB bit numbering (most significant bit first).
 */
template <typename T>
struct MsbBitNumbering
{
    /*! \cond */
    static void bitWrite(T &data, uint8_t bit, bool value)
    {
        Sensors::bitWrite(data, sizeof(T) * CHAR_BIT - 1 - bit, value);
    }
    /*! \endcond */
};

/*!
 * \brief Configuration parameter for BitDecoder that leads to new bits being
 *        added with LSB bit numbering (least significant bit first).
 */
template <typename T>
struct LsbBitNumbering
{
    /*! \cond */
    static void bitWrite(T &data, uint8_t bit, bool value)
    {
        Sensors::bitWrite(data, bit, value);
    }
    /*! \endcond */
};

/*!
 * \brief Configuration parameter for BitDecoder that disables parity checking.
 */
template <typename T>
struct NoParity
{
};

/*!
 * \brief Configuration parameter for BitDecoder that enables even parity
 *        checking.
 */
template <typename T>
struct EvenParity
{
    /*! \cond */
    static bool parityCheck(T data, bool parity)
    {
        return Sensors::parity(data) == parity;
    }
    /*! \endcond */
};

/*!
 * \brief Configuration parameter for BitDecoder that enables odd parity
 *        checking.
 */template <typename T>
struct OddParity
{
    /*! \cond */
    static bool parityCheck(T data, bool parity)
    {
        return Sensors::parity(data) != parity;
    }
    /*! \endcond */
};

/*!
 * \brief BitDecoder base implementation.
 *
 * \attention Implementation detail. This class must not be used directly,
 *            it only serves as super class for BitDecoder.
 */
template <typename T, typename BitDecoder>
class BitDecoderBase
{
public:
    /*!
     * \brief Adds the bit \p value to the BitDecoder state.
     *
     * \param value Bit value to add.
     * \return Decoder state after adding the bit \p value.
     */
    BitDecoderStatus addBit(bool value)
    {
        if (static_cast<BitDecoder *>(this)->isComplete())
            reset();

        return static_cast<BitDecoder *>(this)->internalAddBit(value);
    }

    /*!
     * \brief Returns the converted data.
     *
     * \return The converted data.
     */
    T getData() const
    {
        return m_data;
    }

    /*!
     * \brief Resets the decoder state.
     *
     * This method typically needs to be called when addBit() returned the
     * status BitDecoderStatus::ParityError so that a fresh decoder run starts.
     */
    void reset()
    {
        m_bitLength = 0;
        m_data = 0;
    }

protected:
    BitDecoderBase()
    {
        reset();
    }

    T m_data;
    uint8_t m_bitLength;
};

/*!
 * \brief Applies bit numbering and a parity method to transform continuous bit
 *        streams (for example from RF demodulation) into bytes.
 *
 * Usage:
 * \code
 * using namespace Sensors;
 * BitDecoder<uint8_t, NoParity, MsbBitNumbering> decoder;
 * while (...) {
 *     switch (decoder.addBit(bit)) {
 *     case BitDecoderStatus::Incomplete:
 *         break;
 *     case BitDecoderStatus::Complete:
 *         handle_complete_byte(decoder.getData());
 *         break;
 *     default:
 *         decoder.reset();
 *         break;
 *     }
 * }
 * \endcode
 *
 * \tparam T The data type to convert to (for example uint8_t or uint16_t).
 * \tparam TParity The parity algorithm to be applied
 *         (NoParity, EvenParity, OddParity).
 * \tparam TBitNumbering The bit numbering to be applied
 *         (MsbBitNumbering, LsbBitNumbering).
 *
 * \sa BitDecoderStatus
 */
template <typename T,
          template <typename> class TParity,
          template <typename> class TBitNumbering>
class BitDecoder
        : public BitDecoderBase<T, BitDecoder<T, TParity, TBitNumbering>>
{
    friend class BitDecoderBase<T, BitDecoder<T, TParity, TBitNumbering>>;

    BitDecoderStatus internalAddBit(bool value)
    {
        // Data bit
        if (this->m_bitLength < sizeof(T) * CHAR_BIT) {
            TBitNumbering<T>::bitWrite(this->m_data, this->m_bitLength, value);
        }

        // Parity bit
        else {
            if (!TParity<T>::parityCheck(this->m_data, value)) {
                return BitDecoderStatus::ParityError;
            }
        }

        ++this->m_bitLength;
        return isComplete() ? BitDecoderStatus::Complete :
                              BitDecoderStatus::Incomplete;
    }

    bool isComplete()
    {
        return this->m_bitLength == sizeof(T) * CHAR_BIT + 1;
    }
};

template <typename T,
          template <typename> class TBitNumbering>
class BitDecoder<T, NoParity, TBitNumbering>
        : public BitDecoderBase<T, BitDecoder<T, NoParity, TBitNumbering>>
{
    friend class BitDecoderBase<T, BitDecoder<T, NoParity, TBitNumbering>>;

private:
    BitDecoderStatus internalAddBit(bool value)
    {
        TBitNumbering<T>::bitWrite(this->m_data, this->m_bitLength++, value);
        return isComplete() ? BitDecoderStatus::Complete :
                              BitDecoderStatus::Incomplete;
    }

    bool isComplete()
    {
        return this->m_bitLength == sizeof(T) * CHAR_BIT;
    }
};

/*!
 * \brief A ByteDecoder is a BitDecoder operating on a single data byte.
 */
template <template <typename> class TParity,
          template <typename> class TBitNumbering>
using ByteDecoder = BitDecoder<uint8_t, TParity, TBitNumbering>;

/*! \} */  // \addtogroup libsensors_decoder

}  // namespace Sensors
