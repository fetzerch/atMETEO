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
 * \brief Unit tests for Sensors::HidekiSensor.
 */

#include <gmock/gmock.h>

#include <vector>

#include "lib/hidekisensor.h"

using ::std::vector;
using ::Sensors::SensorStatus;
using ::Sensors::HidekiSensor;

/*!
 * \brief Tests Sensors::HidekiSensor with a correct message.
 */
TEST(HidekiSensorTest, MessageOk)
{
    uint8_t bytes[] = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                       0xC2, 0x16, 0xFB, 0xDB, 0xFC};

    auto sensor = HidekiSensor();
    auto status = sensor.setData(bytes, sizeof(bytes));

    EXPECT_EQ(SensorStatus::Complete, status);
    EXPECT_TRUE(sensor.isValid());
    EXPECT_EQ(1, sensor.channel());
    EXPECT_EQ(0x0C, sensor.sensorId());
    EXPECT_EQ(1, sensor.message());
    EXPECT_TRUE(sensor.isThermoHygro());
    EXPECT_EQ(24, sensor.temperature());
    EXPECT_FLOAT_EQ(24.8, sensor.temperatureF());
    EXPECT_EQ(16, sensor.humidity());
}

/*!
 * \brief Tests Sensors::HidekiSensor with a negative temperature.
 */
TEST(HidekiSensorTest, NegativeTemperature)
{
    uint8_t bytes[] = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                       0x42, 0x16, 0xFB, 0x5B, 0x74};

    auto sensor = HidekiSensor();
    auto status = sensor.setData(bytes, sizeof(bytes));

    EXPECT_EQ(SensorStatus::Complete, status);
    EXPECT_TRUE(sensor.isValid());
    EXPECT_FLOAT_EQ(-24.8, sensor.temperatureF());
}

/*!
 * \brief Tests Sensors::HidekiSensor with continuous data.
 */
TEST(HidekiSensorTest, ContinuousData)
{
    uint8_t bytes[] = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                       0xC2, 0x16, 0xFB, 0xDB, 0xFC};

    auto sensor = HidekiSensor();

    EXPECT_TRUE(sensor.isPossiblyValid());

    for (uint8_t i = 0; i < sizeof(bytes) - 1; ++i) {
        auto status = sensor.addByte(bytes[i]);
        EXPECT_EQ(SensorStatus::Incomplete, status);
    }

    auto status = sensor.addByte(bytes[sizeof(bytes)-1]);
    EXPECT_EQ(SensorStatus::Complete, status);
    EXPECT_TRUE(sensor.isValid());
    EXPECT_EQ(1, sensor.channel());
    EXPECT_EQ(0x0C, sensor.sensorId());
    EXPECT_EQ(1, sensor.message());
    EXPECT_TRUE(sensor.isThermoHygro());
    EXPECT_EQ(24, sensor.temperature());
    EXPECT_FLOAT_EQ(24.8, sensor.temperatureF());
    EXPECT_EQ(16, sensor.humidity());

    status = sensor.addByte(0xFF);
    EXPECT_EQ(SensorStatus::TooMuchData, status);
    EXPECT_TRUE(sensor.isValid());

    sensor.reset();
    status = sensor.addByte(0x8F);
    EXPECT_EQ(SensorStatus::InvalidData, status);
    EXPECT_FALSE(sensor.isPossiblyValid());
}

/*!
 * \brief Tests Sensors::HidekiSensor with an invalid message size.
 */
TEST(HidekiSensorTest, InvalidSize)
{
    uint8_t bytes1[] = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                        0xC2, 0x16, 0xFB, 0xDB};

    uint8_t bytes2[] = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                        0xC2, 0x16, 0xFB, 0xDB, 0xFC, 0xFF};

    auto sensor = HidekiSensor();
    auto status = sensor.setData(bytes1, sizeof(bytes1));

    EXPECT_EQ(SensorStatus::Incomplete, status);
    EXPECT_FALSE(sensor.isValid());

    status = sensor.setData(bytes2, sizeof(bytes2));

    EXPECT_EQ(SensorStatus::TooMuchData, status);
    EXPECT_FALSE(sensor.isValid());
}

/*!
 * \brief Tests Sensors::HidekiSensor with an incorrect header.
 */
TEST(HidekiSensorTest, WrongHeader)
{
    uint8_t bytes[] = {0x00, 0x2C, 0xCE, 0x5E, 0x48,
                       0xC2, 0x16, 0xFB, 0xDB, 0xFC};

    auto sensor = HidekiSensor();
    auto status = sensor.setData(bytes, sizeof(bytes));

    EXPECT_EQ(SensorStatus::InvalidData, status);
    EXPECT_FALSE(sensor.isValid());
}

/*!
 * \brief Tests Sensors::HidekiSensor with an incorrect channel.
 */
TEST(HidekiSensorTest, IncorrectChannel)
{
    uint8_t bytes[] = {0x9F, 0xE3, 0xCE, 0x5E, 0x48,
                       0xC2, 0x16, 0xFB, 0xDB, 0xFC};

    auto sensor = HidekiSensor();
    auto status = sensor.setData(bytes, sizeof(bytes));

    EXPECT_EQ(SensorStatus::InvalidData, status);
    EXPECT_FALSE(sensor.isValid());
}

/*!
 * \brief Tests Sensors::HidekiSensor with an incorrect CRC1.
 */
TEST(HidekiSensorTest, WrongCRC1)
{
    uint8_t bytes[] = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                       0xC2, 0x16, 0xFB, 0x00, 0xFC};

    auto sensor = HidekiSensor();
    auto status = sensor.setData(bytes, sizeof(bytes));

    EXPECT_EQ(SensorStatus::InvalidData, status);
    EXPECT_FALSE(sensor.isValid());
}

/*!
 * \brief Tests Sensors::HidekiSensor with an incorrect CRC2.
 */
TEST(HidekiSensorTest, WrongCRC2)
{
    uint8_t bytes[] = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                       0xC2, 0x16, 0xFB, 0xDB, 0x00};

    auto sensor = HidekiSensor();
    auto status = sensor.setData(bytes, sizeof(bytes));

    EXPECT_EQ(SensorStatus::InvalidData, status);
    EXPECT_FALSE(sensor.isValid());
}

/*!
 * \brief Tests Sensors::HidekiSensor channels.
 */
TEST(HidekiSensorTest, Channels)
{
    uint8_t bytes1[] = {0x9F, 0x24, 0xCE, 0x5E, 0x48,
                        0xC2, 0x16, 0xFB, 0xD3, 0x1E};

    auto sensor = HidekiSensor();
    auto status = sensor.setData(bytes1, sizeof(bytes1));

    EXPECT_EQ(SensorStatus::Complete, status);
    EXPECT_TRUE(sensor.isValid());
    EXPECT_EQ(1, sensor.channel());

    uint8_t bytes2[] = {0x9F, 0x43, 0xCE, 0x5E, 0x48,
                        0xC2, 0x16, 0xFB, 0xB4, 0xC5};

    sensor = HidekiSensor();
    status = sensor.setData(bytes2, sizeof(bytes2));

    EXPECT_EQ(SensorStatus::Complete, status);
    EXPECT_TRUE(sensor.isValid());
    EXPECT_EQ(2, sensor.channel());

    uint8_t bytes3[] = {0x9F, 0x67, 0xCE, 0x5E, 0x48,
                        0xC2, 0x16, 0xFB, 0x90, 0xBE};

    sensor = HidekiSensor();
    status = sensor.setData(bytes3, sizeof(bytes3));

    EXPECT_EQ(SensorStatus::Complete, status);
    EXPECT_TRUE(sensor.isValid());
    EXPECT_EQ(3, sensor.channel());

    uint8_t bytes4[] = {0x9F, 0xAE, 0xCE, 0x5E, 0x48,
                        0xC2, 0x16, 0xFB, 0x59, 0x0C};

    sensor = HidekiSensor();
    status = sensor.setData(bytes4, sizeof(bytes4));

    EXPECT_EQ(SensorStatus::Complete, status);
    EXPECT_TRUE(sensor.isValid());
    EXPECT_EQ(4, sensor.channel());

    uint8_t bytes5[] = {0x9F, 0xC8, 0xCE, 0x5E, 0x48,
                        0xC2, 0x16, 0xFB, 0x3F, 0xBB};

    sensor = HidekiSensor();
    status = sensor.setData(bytes5, sizeof(bytes5));

    EXPECT_EQ(SensorStatus::Complete, status);
    EXPECT_TRUE(sensor.isValid());
    EXPECT_EQ(5, sensor.channel());

    uint8_t bytes6[] = {0x9F, 0x9F, 0xCE, 0x5E, 0x48,
                        0xC2, 0x16, 0xFB, 0x68, 0x6F};

    sensor = HidekiSensor();
    status = sensor.setData(bytes6, sizeof(bytes6));

    EXPECT_EQ(SensorStatus::Complete, status);
    EXPECT_TRUE(sensor.isValid());
    EXPECT_EQ(6, sensor.channel());
}

/*!
 * \brief Tests Sensors::HidekiSensor with a non thermo/hygro message.
 */
TEST(HidekiSensorTest, NonThermoHygroSensor)
{
    // Non Thermo/Hygro sensor type
    uint8_t bytes1[] = {0x9F, 0x2C, 0xCE, 0x5C, 0x48,
                        0xC2, 0x16, 0xFB, 0xD9, 0x71};

    auto sensor = HidekiSensor();
    auto status = sensor.setData(bytes1, sizeof(bytes1));

    EXPECT_EQ(SensorStatus::Complete, status);
    EXPECT_TRUE(sensor.isValid());
    EXPECT_FALSE(sensor.isThermoHygro());
    EXPECT_EQ(0, sensor.temperature());
    EXPECT_FLOAT_EQ(0, sensor.temperatureF());
    EXPECT_EQ(0, sensor.humidity());

    // Thermo/Hygro sensor type, but too small package
    uint8_t bytes2[] = {0x9F, 0x2C, 0xCC, 0x5E, 0x48,
                        0xC2, 0x16, 0x22, 0x36};

    sensor = HidekiSensor();
    status = sensor.setData(bytes2, sizeof(bytes2));

    EXPECT_EQ(SensorStatus::Complete, status);
    EXPECT_TRUE(sensor.isValid());
    EXPECT_FALSE(sensor.isThermoHygro());
    EXPECT_EQ(0, sensor.temperature());
    EXPECT_FLOAT_EQ(0, sensor.temperatureF());
    EXPECT_EQ(0, sensor.humidity());
}
