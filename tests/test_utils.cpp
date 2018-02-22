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

#include <catch.hpp>

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
using ::Sensors::word;
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
TEST_CASE("BitOperationsSimple", "[utils]")
{
    uint8_t byte = 0;

    SECTION("Setting Bits") {
        bitSet(byte, 0);
        CHECK(byte == 0b00000001);
        bitSet(byte, 7);
        CHECK(byte == 0b10000001);
    }

    SECTION("Clearing Bits") {
        byte = 0b00010000;
        bitClear(byte, 4);
        CHECK(byte == 0);
    }

    SECTION("Flipping Bits") {
        byte = 0;
        bitFlip(byte, 7);
        CHECK(byte == 0b10000000);
        bitFlip(byte, 7);
        CHECK(byte == 0);
    }

    SECTION("Reading Bits") {
        byte = 0b11110111;
        CHECK(bitRead(byte, 4) == true);
        CHECK(bitRead(byte, 3) == false);
    }

    SECTION("Writing Bits") {
        byte = 0;
        bitWrite(byte, 0, true);
        CHECK(byte == 0b00000001);
        bitWrite(byte, 0, false);
        CHECK(byte == 0);
    }
}

/*!
 * \brief Tests Sensors::byteReverse.
 */
TEST_CASE("BitOperationsReversingBytes", "[utils]")
{
    CHECK(byteReverse(0)          == 0);
    CHECK(byteReverse(0xFF)       == 0xFF);
    CHECK(byteReverse(0xF0)       == 0x0F);
    CHECK(byteReverse(0x0F)       == 0xF0);
    CHECK(byteReverse(0b10101010) == 0b01010101);
    CHECK(byteReverse(0b10000000) == 0b00000001);
}

/*!
 * \brief Tests Sensors::nibbleReverse.
 */
TEST_CASE("BitOperationsReversingNibbles", "[utils]")
{
    CHECK(nibbleReverse(0)          == 0);
    CHECK(nibbleReverse(0xFF)       == 0xFF);
    CHECK(nibbleReverse(0xF0)       == 0xF0);
    CHECK(nibbleReverse(0x0F)       == 0x0F);
    CHECK(nibbleReverse(0b11000000) == 0b00110000);
    CHECK(nibbleReverse(0b11001100) == 0b00110011);
    CHECK(nibbleReverse(0b00000111) == 0b00001110);
}

/*!
 * \brief Tests Sensors::lowNibble and Sensors::highNibble.
 */
TEST_CASE("BitOperationsLow&HighNibble", "[utils]")
{
    SECTION("Extracting Low Nibble") {
        CHECK(lowNibble(0)          == 0);
        CHECK(lowNibble(0xFF)       == 0x0F);
        CHECK(lowNibble(0xF0)       == 0);
        CHECK(lowNibble(0x0F)       == 0x0F);
        CHECK(lowNibble(0b11000011) == 0b00000011);
    }

    SECTION("Extracting High Nibble") {
        CHECK(highNibble(1)          == 0);
        CHECK(highNibble(0xFF)       == 0x0F);
        CHECK(highNibble(0xF0)       == 0x0F);
        CHECK(highNibble(0x0F)       == 0);
        CHECK(highNibble(0b11000011) == 0b00001100);
    }
}

/*!
 * \brief Tests Sensors::word.
 */
TEST_CASE("BitOperationsCombiningBytesToWord", "[utils]")
{
    CHECK(word(0, 0)       == 0);
    CHECK(word(0xFF, 0xFF) == 0xFFFF);
    CHECK(word(0xAB, 0xCD) == 0xABCD);
}

/*!
 * \brief Tests Sensors::parity.
 */
TEST_CASE("BitOperationsParity", "[utils]")
{
    CHECK(parity(0)          == false);
    CHECK(parity(0xFF)       == false);
    CHECK(parity(0xF0)       == false);
    CHECK(parity(0x0F)       == false);
    CHECK(parity(0b00000001) == true);
    CHECK(parity(0b11111110) == true);
    CHECK(parity(0b11100000) == true);
}

/*!
 * \brief Tests Sensors::min and Sensors::max.
 */
TEST_CASE("BitOperationsMinimum&Maximum", "[utils]")
{
    SECTION("Detecting Minimum") {
        CHECK(min(0, 0) == 0);
        CHECK(min(1, 2) == 1);
        CHECK(min(2, 1) == 1);
    }

    SECTION("Detecting Maximum") {
        CHECK(max(0, 0) == 0);
        CHECK(max(1, 2) == 2);
        CHECK(max(2, 1) == 2);
    }
}
