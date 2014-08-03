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
 * \defgroup libsensors_utils Utilities
 * \ingroup libsensors
 *
 * \brief Bit manipulation utilities.
 *
 * \note All functions are kept as small as possible allowing inlining.
 */

/*!
 * \file
 * \ingroup libsensors_utils
 * \copydoc libsensors_utils
 */

#include <inttypes.h>  // AVR toolchain doesn't offer cinttypes header

namespace Sensors
{

/*!
 * \addtogroup libsensors_utils
 * \{
 */

/*!
 * \brief Sets the \p bit in the given \p value to `1`.
 * \tparam T Data type of the \p value to operate on.
 * \param value Memory location of the \p value where the bit should be set.
 * \param bit Bit position within the \p value.
 */
template <typename T>
inline void bitSet(T &value, uint8_t bit)
{
    value |= (1 << bit);
}

/*!
 * \brief Sets the \p bit in the given \p value to `0`.
 * \tparam T Data type of the \p value to operate on.
 * \param value Memory location of the \p value where the bit should be
 *        cleared.
 * \param bit Bit position within the \p value.
 */
template <typename T>
inline void bitClear(T &value, uint8_t bit)
{
    value &= ~(1 << bit);
}

/*!
 * \brief Flips the \p bit in the given \p value.
 * \tparam T Data type of the \p value to operate on.
 * \param value Memory location of the \p value where bit should be flipped.
 * \param bit Bit position within the \p value.
 */
template <typename T>
inline void bitFlip(T &value, uint8_t bit)
{
    value ^= (1 << bit);
}

/*!
 * \brief Returns the \p bit in the given \p value.
 * \tparam T Data type of the \p value to operate on.
 * \param value Memory location of the \p value where the bit should be read.
 * \param bit Bit position within the \p value.
 * \return Specified bit in the given \p value.
 */
template <typename T>
inline bool bitRead(T &value, uint8_t bit)
{
    return (value >> bit) & 0x01;
}

/*!
 * \brief Writes the \p bit in the given \p value.
 * \tparam T Data type of the \p value to operate on.
 * \param value Memory location of the \p value where the bit should be
 *        written.
 * \param bit Number of the bit to write.
 * \param bitValue Value to write into bit position.
 */
template <typename T>
inline void bitWrite(T &value, uint8_t bit, bool bitValue)
{
    bitValue ? bitSet(value, bit) : bitClear(value, bit);
}

/*!
 * \brief Returns the byte \p x in reversed bit order.
 * \param x The input byte.
 * \return Byte with bits reversed.
 */
inline uint8_t byteReverse(uint8_t x)
{
    x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
    x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
    x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
    return x;
}

/*!
 * \brief Returns the byte \p x in reversed bit order (nibble wise).
 * \param x The input byte.
 * \return Byte with bits nibble wise reversed.
 */
inline uint8_t nibbleReverse(uint8_t x)
{
    x = byteReverse(x);
    x = ((x >> 4) & 0x0F) | ((x << 4) & 0xF0);
    return x;
}

/*!
 * \brief Returns the byte's \p x high nibble.
 * \param x The input byte.
 * \return High nibble of byte \p x.
 */
inline uint8_t highNibble(uint8_t x)
{
    return x >> 4;
}

/*!
 * \brief Returns the byte's \p x low nibble.
 * \param x The input byte.
 * \return Low nibble of byte \p x.
 */
inline uint8_t lowNibble(uint8_t x)
{
    return x & 0x0F;
}

/*!
 * \brief Returns the even parity for the byte \p x.
 *
 * Even parity: Number of set bits modulo 2.
 *
 * \param x The input byte.
 * \return Even parity for the byte \p x.
 */
inline bool parity(int x)
{
    return __builtin_parity(x);
}

/*!
 * \brief Returns the minimum of the two values \p a and \p b.
 * \tparam T Data type of the values to operate on.
 * \param a First value.
 * \param b Second value.
 * \return Minimum of the two values \p a and \p b.
 */
template<typename T>
inline T min(T a, T b)
{
    return a < b ? a : b;
}

/*!
 * \brief Returns the maximum of the two values \p a and \p b.
 * \tparam T Data type of the values to operate on.
 * \param a First value.
 * \param b Second value.
 * \return Maximum of the two values \p a and \p b.
 */
template<typename T>
inline T max(T a, T b)
{
    return a > b ? a : b;
}

/*! \} */  // \addtogroup libsensors_utils

}  // namespace Sensors
