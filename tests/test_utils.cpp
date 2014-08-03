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

/*!
 * \file
 * \ingroup libsensors_tests
 *
 * \brief Unit tests for \ref libsensors_utils.
 */

#include <gtest/gtest.h>

#include "lib/utils.h"

using ::Sensors::bitSet;
using ::Sensors::bitClear;
using ::Sensors::bitFlip;
using ::Sensors::bitRead;
using ::Sensors::bitWrite;
using ::Sensors::byteReverse;
using ::Sensors::nibbleReverse;
using ::Sensors::lowNibble;
using ::Sensors::highNibble;
using ::Sensors::parity;
using ::Sensors::min;
using ::Sensors::max;

/*!
 * \brief Tests bit operations.
 * \sa Sensors::bitSet
 * \sa Sensors::bitClear
 * \sa Sensors::bitFlip
 * \sa Sensors::bitRead
 * \sa Sensors::bitWrite
 */
TEST(UtilsTest, BitOperations)
{
    uint8_t byte = 0;

    bitSet(byte, 0);
    EXPECT_EQ(0b00000001, byte);

    bitSet(byte, 7);
    EXPECT_EQ(0b10000001, byte);

    byte = 0b00010000;
    bitClear(byte, 4);
    EXPECT_EQ(0, byte);

    byte = 0;
    bitFlip(byte, 7);
    EXPECT_EQ(0b10000000, byte);
    bitFlip(byte, 7);
    EXPECT_EQ(0, byte);

    byte = 0b11110111;
    EXPECT_TRUE(bitRead(byte, 4));
    EXPECT_FALSE(bitRead(byte, 3));

    byte = 0;
    bitWrite(byte, 0, true);
    EXPECT_EQ(0b00000001, byte);
    bitWrite(byte, 0, false);
    EXPECT_EQ(0, byte);
}

/*!
 * \brief Tests Sensors::byteReverse.
 */
TEST(UtilsTest, ByteReverse)
{
    EXPECT_EQ(0,    byteReverse(0));
    EXPECT_EQ(0xFF, byteReverse(0xFF));
    EXPECT_EQ(0x0F, byteReverse(0xF0));
    EXPECT_EQ(0xF0, byteReverse(0x0F));
    EXPECT_EQ(0b01010101, byteReverse(0b10101010));
    EXPECT_EQ(0b00000001, byteReverse(0b10000000));
}

/*!
 * \brief Tests Sensors::nibbleReverse.
 */
TEST(UtilsTest, NibbleReverse)
{
    EXPECT_EQ(0,    nibbleReverse(0));
    EXPECT_EQ(0xFF, nibbleReverse(0xFF));
    EXPECT_EQ(0xF0, nibbleReverse(0xF0));
    EXPECT_EQ(0x0F, nibbleReverse(0x0F));
    EXPECT_EQ(0b00110000, nibbleReverse(0b11000000));
    EXPECT_EQ(0b00110011, nibbleReverse(0b11001100));
    EXPECT_EQ(0b00001110, nibbleReverse(0b00000111));
}

/*!
 * \brief Tests Sensors::lowNibble and Sensors::highNibble.
 */
TEST(UtilsTest, Nibbles)
{
    EXPECT_EQ(0,    lowNibble(0));
    EXPECT_EQ(0x0F, lowNibble(0xFF));
    EXPECT_EQ(0,    lowNibble(0xF0));
    EXPECT_EQ(0x0F, lowNibble(0x0F));
    EXPECT_EQ(0b00000011, lowNibble(0b11000011));

    EXPECT_EQ(0,    highNibble(0));
    EXPECT_EQ(0x0F, highNibble(0xFF));
    EXPECT_EQ(0x0F, highNibble(0xF0));
    EXPECT_EQ(0,    highNibble(0x0F));
    EXPECT_EQ(0b00001100, highNibble(0b11000011));
}

/*!
 * \brief Tests Sensors::parity.
 */
TEST(UtilsTest, Parity)
{
    uint8_t byte;

    byte = 0;
    EXPECT_FALSE(parity(byte));

    byte = 0xFF;
    EXPECT_FALSE(parity(byte));

    byte = 0xF0;
    EXPECT_FALSE(parity(byte));

    byte = 0x0F;
    EXPECT_FALSE(parity(byte));

    byte = 0b00000001;
    EXPECT_TRUE(parity(byte));

    byte = 0b11111110;
    EXPECT_TRUE(parity(byte));

    byte = 0b11100000;
    EXPECT_TRUE(parity(byte));
}

/*!
 * \brief Tests Sensors::min and Sensors::max.
 */
TEST(UtilsTest, MinMax)
{
    EXPECT_EQ(0, min(0, 0));
    EXPECT_EQ(1, min(1, 2));
    EXPECT_EQ(1, min(2, 1));

    EXPECT_EQ(0, max(0, 0));
    EXPECT_EQ(2, max(1, 2));
    EXPECT_EQ(2, max(2, 1));
}
