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
 * \brief Unit tests for \ref libsensors_bitdecoder.
 */

#include <vector>

#include <catch.hpp>

#include "lib/utils.h"
#include "lib/bitdecoder.h"

using ::Catch::Matchers::Equals;
using ::Sensors::BitDecoder;
using ::Sensors::BitDecoderStatus;
using ::Sensors::NoParity;
using ::Sensors::EvenParity;
using ::Sensors::OddParity;
using ::Sensors::MsbBitNumbering;
using ::Sensors::LsbBitNumbering;

template <template <typename> class TParity>
struct ParityHelper {};

template <>
struct ParityHelper<Sensors::EvenParity>
{
    static const bool parity = true;
};

template <>
struct ParityHelper<Sensors::OddParity>
{
    static const bool parity = false;
};

template <template <typename> class TBitNumbering>
struct BitNumberingHelper {};

template <>
struct BitNumberingHelper<Sensors::MsbBitNumbering>
{
    static const uint64_t value = 0xAAAAAAAAAAAAAAAA;
};

template <>
struct BitNumberingHelper<Sensors::LsbBitNumbering>
{
    static const uint64_t value = 0x5555555555555555;
};

template <typename T,
          template <typename> class TParity,
          template <typename> class TBitNumbering>
struct BitDecoderTestHelper
{
    static void test() {
        BitDecoder<T, TParity, TBitNumbering> decoder;
        BitDecoderStatus status;
        for (size_t i = 0; i < 2 * sizeof(T) * CHAR_BIT + 2; ++i) {
            if (i == sizeof(T) * CHAR_BIT) {
                status = decoder.addBit(!ParityHelper<TParity>::parity);
                CHECK(status == BitDecoderStatus::Complete);
                CHECK(decoder.getData() ==
                      static_cast<T>(
                          BitNumberingHelper<TBitNumbering>::value));
            } else if (i == 2 * sizeof(T) * CHAR_BIT + 1) {
                status = decoder.addBit(ParityHelper<TParity>::parity);
                CHECK(status == BitDecoderStatus::ParityError);
            } else {
                status = decoder.addBit(i % 2 == 0);
                CHECK(status == BitDecoderStatus::Incomplete);
            }
        }
    }
};

template <typename T,
          template <typename> class TBitNumbering>
struct BitDecoderTestHelper<T, Sensors::NoParity, TBitNumbering>
{
    static void test() {
        BitDecoder<T, Sensors::NoParity, TBitNumbering> decoder;
        BitDecoderStatus status;
        for (size_t i = 0; i < 2 * sizeof(T) * CHAR_BIT; ++i) {
            if (i == sizeof(T) * CHAR_BIT - 1 ||
                    i == 2 * sizeof(T) * CHAR_BIT - 1) {
                status = decoder.addBit(0);
                CHECK(status == BitDecoderStatus::Complete);
                CHECK(decoder.getData() ==
                      static_cast<T>(
                          BitNumberingHelper<TBitNumbering>::value));
            } else {
                status = decoder.addBit(i % 2 == 0);
                CHECK(BitDecoderStatus::Incomplete == status);
            }
        }
    }
};

/*!
 * \brief Tests Sensors::BitDecoder with uint8_t, Sensors::NoParity
 *        and Sensors::MsbBitNumbering configuration.
 */
TEST_CASE("DecodingBitsWithByteNoParityMsb", "[bitdecoder]")
{
    BitDecoderTestHelper<uint8_t, NoParity, MsbBitNumbering>::test();
}

/*!
 * \brief Tests Sensors::BitDecoder with uint8_t, Sensors::NoParity
 *        and Sensors::LsbBitNumbering configuration.
 */
TEST_CASE("DecodingBitsWithByteNoParityLsb", "[bitdecoder]")
{
    BitDecoderTestHelper<uint8_t, NoParity, LsbBitNumbering>::test();
}

/*!
 * \brief Tests Sensors::BitDecoder with uint8_t, Sensors::EvenParity
 *        and Sensors::MsbBitNumbering configuration.
 */
TEST_CASE("DecodingBitsWithByteEvenParityMsb", "[bitdecoder]")
{
    BitDecoderTestHelper<uint8_t, EvenParity, MsbBitNumbering>::test();
}

/*!
 * \brief Tests Sensors::BitDecoder with uint8_t, Sensors::EvenParity
 *        and Sensors::LsbBitNumbering configuration.
 */
TEST_CASE("DecodingBitsWithByteEvenParityLsb", "[bitdecoder]")
{
    BitDecoderTestHelper<uint8_t, EvenParity, LsbBitNumbering>::test();
}

/*!
 * \brief Tests Sensors::BitDecoder with uint8_t, Sensors::OddParity
 *        and Sensors::MsbBitNumbering configuration.
 */
TEST_CASE("DecodingBitsWithByteOddParityMsb", "[bitdecoder]")
{
    BitDecoderTestHelper<uint8_t, OddParity, MsbBitNumbering>::test();
}

/*!
 * \brief Tests Sensors::BitDecoder with uint8_t, Sensors::OddParity
 *        and Sensors::LsbBitNumbering configuration.
 */
TEST_CASE("DecodingBitsWithByteOddParityLsb", "[bitdecoder]")
{
    BitDecoderTestHelper<uint8_t, OddParity, LsbBitNumbering>::test();
}

/*!
 * \brief Tests Sensors::BitDecoder with uint32_t, Sensors::NoParity
 *        and Sensors::MsbBitNumbering configuration.
 */
TEST_CASE("DecodingBitsWithIntNoParityMsb", "[bitdecoder]")
{
    BitDecoderTestHelper<uint32_t, NoParity, MsbBitNumbering>::test();
}

/*!
 * \brief Tests Sensors::BitDecoder with uint32_t, Sensors::NoParity
 *        and Sensors::LsbBitNumbering configuration.
 */
TEST_CASE("DecodingBitsWithIntNoParityLsb", "[bitdecoder]")
{
    BitDecoderTestHelper<uint32_t, NoParity, LsbBitNumbering>::test();
}

/*!
 * \brief Tests Sensors::BitDecoder with uint32_t, Sensors::EvenParity
 *        and Sensors::MsbBitNumbering configuration.
 */
TEST_CASE("DecodingBitsWithIntEvenParityMsb", "[bitdecoder]")
{
    BitDecoderTestHelper<uint32_t, EvenParity, MsbBitNumbering>::test();
}

/*!
 * \brief Tests Sensors::BitDecoder with uint32_t, Sensors::EvenParity
 *        and Sensors::LsbBitNumbering configuration.
 */
TEST_CASE("DecodingBitsWithIntEvenParityLsb", "[bitdecoder]")
{
    BitDecoderTestHelper<uint32_t, EvenParity, LsbBitNumbering>::test();
}

/*!
 * \brief Tests Sensors::BitDecoder with uint32_t, Sensors::OddParity
 *        and Sensors::MsbBitNumbering configuration.
 */
TEST_CASE("DecodingBitsWithIntOddParityMsb", "[bitdecoder]")
{
    BitDecoderTestHelper<uint32_t, OddParity, MsbBitNumbering>::test();
}

/*!
 * \brief Tests Sensors::BitDecoder with uint32_t, Sensors::OddParity
 *        and Sensors::LsbBitNumbering configuration.
 */
TEST_CASE("DecodingBitsWithIntOddParityLsb", "[bitdecoder]")
{
    BitDecoderTestHelper<uint32_t, OddParity, LsbBitNumbering>::test();
}

/*!
 * \brief Tests Sensors::BitDecoder with a recorded RF message (using
 *        uint32_t, Sensors::EvenParity and Sensors::LsbBitNumbering
 *        configuration).
 */
TEST_CASE("DecodingBitsFromRecordedRFMessage", "[bitdecoder]")
{
    uint8_t message[] = {1, 1, 1, 1, 1, 0, 0, 1, 0,
                         0, 0, 1, 1, 0, 1, 0, 0, 1,
                         0, 1, 1, 1, 0, 0, 1, 1, 1,
                         0, 1, 1, 1, 1, 0, 1, 0, 1,
                         0, 0, 0, 1, 0, 0, 1, 0, 0,
                         0, 1, 0, 0, 0, 0, 1, 1, 1,
                         0, 1, 1, 0, 1, 0, 0, 0, 1,
                         1, 1, 0, 1, 1, 1, 1, 1, 1,
                         1, 1, 0, 1, 1, 0, 1, 1, 0,
                         0, 0, 1, 1, 1, 1, 1, 1,
                         1, 1, 1, 1, 1, 0, 0, 1, 0,
                         0, 0, 1, 1, 0, 1, 0, 0, 1,
                         0, 1, 1, 1, 0, 0, 1, 1, 1,
                         0, 1, 1, 1, 1, 0, 0, 1, 1,
                         0, 0, 0, 1, 0, 0, 1, 0, 0,
                         0, 1, 0, 0, 0, 0, 1, 1, 1,
                         0, 1, 1, 0, 1, 0, 0, 0, 1,
                         1, 1, 0, 1, 1, 1, 1, 1, 1,
                         1, 1, 0, 1, 1, 0, 0, 0, 0,
                         0, 1, 0, 0, 1, 1, 0, 1,
                         1, 1, 1, 1, 1, 0, 0, 1, 0,
                         0, 0, 1, 1, 0, 1, 0, 0, 1,
                         0, 1, 1, 1, 0, 0, 1, 1, 1,
                         0, 1, 1, 1, 1, 0, 1, 1, 0,
                         0, 0, 0, 1, 0, 0, 1, 0, 0,
                         0, 1, 0, 0, 0, 0, 1, 1, 1,
                         0, 1, 1, 0, 1, 0, 0, 0, 1,
                         1, 1, 0, 1, 1, 1, 1, 1, 1,
                         1, 1, 0, 1, 1, 0, 1, 0, 1,
                         0, 0, 0, 1, 0, 0, 0, 1};

    std::vector<uint8_t> result = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                        0xC2, 0x16, 0xFB, 0xDB, 0xFC,
                        0x9F, 0x2C, 0xCE, 0x9E, 0x48,
                        0xC2, 0x16, 0xFB, 0x1B, 0xB2,
                        0x9F, 0x2C, 0xCE, 0xDE, 0x48,
                        0xC2, 0x16, 0xFB, 0x5B, 0x88};

    std::vector<uint8_t> bytes;
    BitDecoder<uint8_t, EvenParity, LsbBitNumbering> decoder;

    for (uint16_t i = 0; i < std::extent<decltype(message)>::value; ++i) {
        auto status = decoder.addBit(message[i]);

        // The parity bit for the last bytes in each of the 3 messages is
        // missing. Thus the status in that case won't indicate completion.
        if (status == BitDecoderStatus::Complete || (i % 89 == 88)) {
            bytes.emplace_back(decoder.getData());
            decoder.reset();
        }
    }

    CHECK_THAT(bytes, Equals(result));
}
