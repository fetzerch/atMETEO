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
 * \ingroup libsensors_tests
 *
 * \brief Testing utilities
 *
 * This header file provides helpful utilities for testing.
 *
 * \note Being part of the unit tests the utils can make use of STL.
 */

#include <sstream>
#include <iomanip>

/*!
 * \addtogroup libsensors_tests
 * \{
 */

/*!
 * \brief Converts the numeric \p value into its hex representation.
 * \tparam T Data type of the numeric \p value to operate on.
 * \param value Numeric \p value.
 * \return Hex representation of the \p value.
 */
template <typename T>
std::string to_hex(T value)
{
    std::stringstream s;
    s << std::showbase << std::setfill('0') << std::setw(sizeof(T) * 2)
      << std::hex << static_cast<int>(value);
    return s.str();
}

/*!
 * \brief Converts the array of numeric values \p data into its hex
 *        representation.
 * \tparam T Data type of the elements to operate on.
 * \param data Array of numeric values.
 * \param length Length of the array.
 * \return Hex representation of the numeric values in the array \p data.
 */
template <typename T>
std::string to_hex(T *data, uint16_t length)
{
    std::stringstream s;
    for (uint16_t i = 0; i < length; ++i) {
        s << to_hex(data[i]) << " ";
    }
    return s.str();
}

/*! \} */
