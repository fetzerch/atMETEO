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
 * \brief Unit tests for Sensors::Tgs2600.
 */

#include <limits>
#include <tuple>
#include <memory>

#include <gtest/gtest.h>

#include "lib/tgs2600.h"

using ::testing::Values;
using ::testing::TestWithParam;

using ::Sensors::Tgs2600;

/*! \cond */
using TestTgs2600 = Tgs2600<10000>;
/*! \endcond */

class Tgs2600Test : public TestWithParam<std::tuple<uint32_t, uint32_t>>
{
};

/*!
 * \brief Parameterized test for Sensors::Tgs2600::sensorResistance().
 */
TEST_P(Tgs2600Test, Uncalibrated)
{
    TestTgs2600 tgs2600;
    EXPECT_EQ(std::get<1>(GetParam()),
              tgs2600.sensorResistance(std::get<0>(GetParam())));
}

/*!
 * \brief Tests Sensors::Tgs2600::calibration().
 */
TEST(Tgs2600Test, CalibrationInit)
{
    TestTgs2600 tgs2600;

    // With reference (37%, 20°C, factor 0.7824)
    tgs2600.setReferenceHumidity(37);
    tgs2600.setReferenceTemperature(20);
    auto calibration = tgs2600.calibration();
    EXPECT_FLOAT_EQ(0.7824, calibration);
}

/*!
 * \brief Parameterized test for Sensors::Tgs2600::sensorResistanceCalibrated().
 */
TEST_P(Tgs2600Test, Calibrated)
{
    TestTgs2600 tgs2600;

    // With reference (37%, 20°C, factor 0.7824)
    tgs2600.setReferenceHumidity(37);
    tgs2600.setReferenceTemperature(20);

    EXPECT_EQ(static_cast<uint32_t>(
                  std::get<1>(GetParam()) * tgs2600.calibration()),
              tgs2600.sensorResistanceCalibrated(std::get<0>(GetParam())));
}

/*!
 * \brief Parameterized test for Sensors::Tgs2600::sensorResistanceRelative().
 */
TEST_P(Tgs2600Test, Relative)
{
    TestTgs2600 tgs2600;

    float referenceResistance = 2500.0F;
    tgs2600.setReferenceResistance(referenceResistance);

    EXPECT_EQ(std::get<1>(GetParam()) / referenceResistance,
              tgs2600.sensorResistanceRelative(std::get<0>(GetParam())));
}

/*!
 * \brief Parameterized test instantiation for
 *        \ref TEST_P(Tgs2600Test, Uncalibrated),
 *        \ref TEST_P(Tgs2600Test, Calibrated) and
 *        \ref TEST_P(Tgs2600Test, Relative).
 */
INSTANTIATE_TEST_CASE_P(Samples, Tgs2600Test, Values(
    std::make_tuple( 500, 90000),
    std::make_tuple(1000, 40000),
    std::make_tuple(2000, 15000),
    std::make_tuple(3000,  6666),
    std::make_tuple(4000,  2500),
    std::make_tuple(5000,     0),
    std::make_tuple(8000,     0)
));

/*!
 * \brief Tests Sensors::Tgs2600::sensorResistance() and
 *        Sensors::Tgs2600::sensorResistanceCalibrated() on an ADC voltage drop.
 */
TEST(Tgs2600Test, AdcVoltageDrop)
{
    TestTgs2600 tgs2600;

    tgs2600.setReferenceHumidity(37);
    tgs2600.setReferenceTemperature(20);

    EXPECT_EQ(UINT32_MAX, tgs2600.sensorResistance(0));
    EXPECT_EQ(UINT32_MAX, tgs2600.sensorResistanceCalibrated(0));
}

/*!
 * \brief Tests mathematical limits of Sensors::Tgs2600::sensorResistance() and
 *        Sensors::Tgs2600::sensorResistanceCalibrated().
 */
TEST(Tgs2600Test, Limits)
{
    {
        Tgs2600<UINT32_MAX / 5000> tgs2600;
        EXPECT_EQ(UINT32_MAX, tgs2600.sensorResistance(0));
        EXPECT_EQ(4294106007U, tgs2600.sensorResistance(1));
        EXPECT_EQ(171U, tgs2600.sensorResistance(4999));
        EXPECT_EQ(0U, tgs2600.sensorResistance(5000));
        EXPECT_EQ(0U, tgs2600.sensorResistance(UINT16_MAX));
    }
    {
        Tgs2600<450> tgs2600;
        EXPECT_EQ(UINT32_MAX, tgs2600.sensorResistance(0));
        EXPECT_EQ(2249550U, tgs2600.sensorResistance(1));
        EXPECT_EQ(0U, tgs2600.sensorResistance(4999));
        EXPECT_EQ(0U, tgs2600.sensorResistance(5000));
        EXPECT_EQ(0U, tgs2600.sensorResistance(UINT16_MAX));
    }
}
