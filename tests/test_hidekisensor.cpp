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

#include <vector>

#include <catch.hpp>

#include "lib/hidekisensor.h"

using ::Sensors::SensorStatus;
using ::Sensors::HidekiSensor;

/*!
 * \brief Tests Sensors::HidekiSensor with a correct message.
 */
TEST_CASE("HidekiSensorCorrectMessage", "[hidekisensor]")
{
    std::vector<uint8_t> bytes = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                                  0xC2, 0x16, 0xFB, 0xDB, 0xFC};

    auto sensor = HidekiSensor();
    auto status = sensor.setData(bytes.data(), bytes.size());

    CHECK(status == SensorStatus::Complete);
    CHECK(sensor.isValid());
    CHECK(sensor.channel() == 1);
    CHECK(sensor.sensorId() == 0x0C);
    CHECK(sensor.message() == 1);
    CHECK(sensor.isThermoHygro());
    CHECK(sensor.temperature() == 24);
    CHECK(sensor.temperatureF() == Approx(24.8));
    CHECK(sensor.humidity() == 16);
}

/*!
 * \brief Tests Sensors::HidekiSensor with a negative temperature.
 */
TEST_CASE("HidekiSensorNegativeTemperature", "[hidekisensor]")
{
    std::vector<uint8_t> bytes = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                                  0x42, 0x16, 0xFB, 0x5B, 0x74};

    auto sensor = HidekiSensor();
    auto status = sensor.setData(bytes.data(), bytes.size());

    CHECK(status == SensorStatus::Complete);
    CHECK(sensor.isValid());
    CHECK(sensor.temperatureF() == Approx(-24.8));
}

/*!
 * \brief Tests Sensors::HidekiSensor with continuous data.
 */
TEST_CASE("HidekiSensorContinuousData", "[hidekisensor]")
{
    std::vector<uint8_t> bytes = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                                  0xC2, 0x16, 0xFB, 0xDB, 0xFC};

    auto sensor = HidekiSensor();

    CHECK(sensor.isPossiblyValid());

    for (uint8_t i = 0; i < bytes.size() - 1; ++i) {
        auto status = sensor.addByte(bytes[i]);
        CHECK(status == SensorStatus::Incomplete);
    }

    auto status = sensor.addByte(bytes[bytes.size()-1]);
    CHECK(status == SensorStatus::Complete);
    CHECK(sensor.isValid());
    CHECK(sensor.channel() == 1);
    CHECK(sensor.sensorId() == 0x0C);
    CHECK(sensor.message() == 1);
    CHECK(sensor.isThermoHygro());
    CHECK(sensor.temperature() == 24);
    CHECK(sensor.temperatureF() == Approx(24.8));
    CHECK(sensor.humidity() == 16);

    status = sensor.addByte(0xFF);
    CHECK(status == SensorStatus::TooMuchData);
    CHECK(sensor.isValid());

    sensor.reset();
    status = sensor.addByte(0x8F);
    CHECK(status == SensorStatus::InvalidData);
    CHECK(sensor.isPossiblyValid() == false);
}

/*!
 * \brief Tests Sensors::HidekiSensor with an invalid message size.
 */
TEST_CASE("HidekiSensorInvalidSize", "[hidekisensor]")
{
    auto sensor = HidekiSensor();

    SECTION("Incomplete Message") {
        std::vector<uint8_t> bytes = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                                      0xC2, 0x16, 0xFB, 0xDB};

        auto status = sensor.setData(bytes.data(), bytes.size());
        CHECK(status == SensorStatus::Incomplete);
        CHECK(sensor.isValid() == false);
    }

    SECTION("Too Much Data in Message") {
        std::vector<uint8_t> bytes = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                                      0xC2, 0x16, 0xFB, 0xDB, 0xFC, 0xFF};

        auto status = sensor.setData(bytes.data(), bytes.size());
        CHECK(status == SensorStatus::TooMuchData);
        CHECK(sensor.isValid() == false);
    }
}

/*!
 * \brief Tests Sensors::HidekiSensor with an incorrect header.
 */
TEST_CASE("HidekiSensorIncorrectHeader", "[hidekisensor]")
{
    std::vector<uint8_t> bytes = {0x00, 0x2C, 0xCE, 0x5E, 0x48,
                                  0xC2, 0x16, 0xFB, 0xDB, 0xFC};

    auto sensor = HidekiSensor();
    auto status = sensor.setData(bytes.data(), bytes.size());

    CHECK(status == SensorStatus::InvalidData);
    CHECK(sensor.isValid() == false);
}

/*!
 * \brief Tests Sensors::HidekiSensor with an incorrect channel.
 */
TEST_CASE("HidekiSensorIncorrectChannel", "[hidekisensor]")
{
    std::vector<uint8_t> bytes = {0x9F, 0xE3, 0xCE, 0x5E, 0x48,
                                  0xC2, 0x16, 0xFB, 0xDB, 0xFC};

    auto sensor = HidekiSensor();
    auto status = sensor.setData(bytes.data(), bytes.size());

    CHECK(status == SensorStatus::InvalidData);
    CHECK(sensor.isValid() == false);
}

/*!
 * \brief Tests Sensors::HidekiSensor with an incorrect CRC1.
 */
TEST_CASE("HidekiSensorIncorrectCRC1", "[hidekisensor]")
{
    std::vector<uint8_t> bytes = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                                  0xC2, 0x16, 0xFB, 0x00, 0xFC};

    auto sensor = HidekiSensor();
    auto status = sensor.setData(bytes.data(), bytes.size());

    CHECK(status == SensorStatus::InvalidData);
    CHECK(sensor.isValid() == false);
}

/*!
 * \brief Tests Sensors::HidekiSensor with an incorrect CRC2.
 */
TEST_CASE("HidekiSensorIncorrectCRC2", "[hidekisensor]")
{
    std::vector<uint8_t> bytes = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                                  0xC2, 0x16, 0xFB, 0xDB, 0x00};

    auto sensor = HidekiSensor();
    auto status = sensor.setData(bytes.data(), bytes.size());

    CHECK(status == SensorStatus::InvalidData);
    CHECK(sensor.isValid() == false);
}

/*!
 * \brief Tests Sensors::HidekiSensor channels.
 */
TEST_CASE("HidekiSensorChannels", "[hidekisensor]")
{
    auto sensor = HidekiSensor();

    SECTION("Channel 1") {
        std::vector<uint8_t> bytes = {0x9F, 0x24, 0xCE, 0x5E, 0x48,
                                      0xC2, 0x16, 0xFB, 0xD3, 0x1E};

        auto status = sensor.setData(bytes.data(), bytes.size());
        CHECK(status == SensorStatus::Complete);
        CHECK(sensor.isValid());
        CHECK(sensor.channel() == 1);
    }

    SECTION("Channel 2") {
        std::vector<uint8_t> bytes = {0x9F, 0x43, 0xCE, 0x5E, 0x48,
                                      0xC2, 0x16, 0xFB, 0xB4, 0xC5};

        auto status = sensor.setData(bytes.data(), bytes.size());
        CHECK(status == SensorStatus::Complete);
        CHECK(sensor.isValid());
        CHECK(sensor.channel() == 2);
    }

    SECTION("Channel 3") {
        std::vector<uint8_t> bytes = {0x9F, 0x67, 0xCE, 0x5E, 0x48,
                                      0xC2, 0x16, 0xFB, 0x90, 0xBE};

        auto status = sensor.setData(bytes.data(), bytes.size());
        CHECK(status == SensorStatus::Complete);
        CHECK(sensor.isValid());
        CHECK(sensor.channel() == 3);
    }

    SECTION("Channel 4") {
        std::vector<uint8_t> bytes = {0x9F, 0xAE, 0xCE, 0x5E, 0x48,
                                      0xC2, 0x16, 0xFB, 0x59, 0x0C};

        auto status = sensor.setData(bytes.data(), bytes.size());
        CHECK(status == SensorStatus::Complete);
        CHECK(sensor.isValid());
        CHECK(sensor.channel() == 4);
    }

    SECTION("Channel 5") {
        std::vector<uint8_t> bytes = {0x9F, 0xC8, 0xCE, 0x5E, 0x48,
                                      0xC2, 0x16, 0xFB, 0x3F, 0xBB};

        auto status = sensor.setData(bytes.data(), bytes.size());
        CHECK(status == SensorStatus::Complete);
        CHECK(sensor.isValid());
        CHECK(sensor.channel() == 5);
    }

    SECTION("Channel 6") {
        std::vector<uint8_t> bytes = {0x9F, 0x9F, 0xCE, 0x5E, 0x48,
                                      0xC2, 0x16, 0xFB, 0x68, 0x6F};

        auto status = sensor.setData(bytes.data(), bytes.size());
        CHECK(status == SensorStatus::Complete);
        CHECK(sensor.isValid());
        CHECK(sensor.channel() == 6);
    }
}

/*!
 * \brief Tests Sensors::HidekiSensor battery status.
 */
TEST_CASE("HidekiSensorBatteryStatus", "[hidekisensor]")
{
    auto sensor = HidekiSensor();

    SECTION("Battery OK") {
        std::vector<uint8_t> bytes = {0x9F, 0x2C, 0xCE, 0x5E, 0x48,
                                      0xC2, 0x16, 0xFB, 0xDB, 0xFC};

        auto status = sensor.setData(bytes.data(), bytes.size());
        CHECK(status == SensorStatus::Complete);
        CHECK(sensor.isValid());
        CHECK(sensor.batteryOk() == true);
    }

    SECTION("Battery Not OK") {
        std::vector<uint8_t> bytes = {0x9F, 0x2C, 0x0E, 0x5E, 0x48,
                                      0xC2, 0x16, 0xFB, 0x1B, 0x0A};

        auto status = sensor.setData(bytes.data(), bytes.size());
        CHECK(status == SensorStatus::Complete);
        CHECK(sensor.isValid());
        CHECK(sensor.batteryOk() == false);
    }
}

/*!
 * \brief Tests Sensors::HidekiSensor with a non thermo/hygro message.
 */
TEST_CASE("HidekiSensorNonThermo/HygroSensor", "[hidekisensor]")
{
    auto sensor = HidekiSensor();

    SECTION("Non Thermo/Hygro Sensor Type") {
        std::vector<uint8_t> bytes = {0x9F, 0x2C, 0xCE, 0x5C, 0x48,
                                      0xC2, 0x16, 0xFB, 0xD9, 0x71};

        auto status = sensor.setData(bytes.data(), bytes.size());
        CHECK(status == SensorStatus::Complete);
        CHECK(sensor.isValid());
        CHECK(sensor.isThermoHygro() == false);
        CHECK(sensor.temperature() == 0);
        CHECK(sensor.temperatureF() == Approx(0));
        CHECK(sensor.humidity() == 0);
    }

    SECTION("Thermo/Hygro Sensor With Too Small Package") {
        std::vector<uint8_t> bytes = {0x9F, 0x2C, 0xCC, 0x5E, 0x48,
                                      0xC2, 0x16, 0x22, 0x36};

        auto status = sensor.setData(bytes.data(), bytes.size());
        CHECK(status == SensorStatus::Complete);
        CHECK(sensor.isValid());
        CHECK(sensor.isThermoHygro() == false);
        CHECK(sensor.temperature() == 0);
        CHECK(sensor.temperatureF() == Approx(0));
        CHECK(sensor.humidity() == 0);
    }
}
