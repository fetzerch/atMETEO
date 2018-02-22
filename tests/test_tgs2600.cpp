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

#include <catch.hpp>

#include "lib/tgs2600.h"

using ::Sensors::Tgs2600;

/*! \cond */
using TestTgs2600 = Tgs2600<10000>;
/*! \endcond */

/*!
 * \brief Parameterized test for Sensors::Tgs2600::sensorResistance().
 */
TEST_CASE("Tgs2600Uncalibrated", "[tgs2600]")
{
    TestTgs2600 tgs2600;

    CHECK(tgs2600.sensorResistance( 500) == 90000);
    CHECK(tgs2600.sensorResistance(1000) == 40000);
    CHECK(tgs2600.sensorResistance(2000) == 15000);
    CHECK(tgs2600.sensorResistance(3000) ==  6666);
    CHECK(tgs2600.sensorResistance(4000) ==  2500);
    CHECK(tgs2600.sensorResistance(5000) ==     0);
    CHECK(tgs2600.sensorResistance(8000) ==     0);
}

/*!
 * \brief Tests Sensors::Tgs2600::calibration().
 */
TEST_CASE("Tgs2600Calibration", "[tgs2600]")
{
    TestTgs2600 tgs2600;

    // With reference (37%, 20°C, factor 0.7824)
    tgs2600.setReferenceHumidity(37);
    tgs2600.setReferenceTemperature(20);
    CHECK(tgs2600.calibration() == Approx(0.7824));
}

/*!
 * \brief Parameterized test for Sensors::Tgs2600::sensorResistanceCalibrated().
 */
TEST_CASE("Tgs2600CalibratedResistance", "[tgs2600]")
{
    TestTgs2600 tgs2600;

    // With reference (37%, 20°C, factor 0.7824)
    tgs2600.setReferenceHumidity(37);
    tgs2600.setReferenceTemperature(20);

    CHECK(tgs2600.sensorResistanceCalibrated( 500) == 70416);
    CHECK(tgs2600.sensorResistanceCalibrated(1000) == 31296);
    CHECK(tgs2600.sensorResistanceCalibrated(2000) == 11736);
    CHECK(tgs2600.sensorResistanceCalibrated(3000) ==  5215);
    CHECK(tgs2600.sensorResistanceCalibrated(4000) ==  1956);
    CHECK(tgs2600.sensorResistanceCalibrated(5000) ==     0);
    CHECK(tgs2600.sensorResistanceCalibrated(8000) ==     0);
}

/*!
 * \brief Parameterized test for Sensors::Tgs2600::sensorResistanceRelative().
 */
TEST_CASE("Tgs2600RelativeResistance", "[tgs2600]")
{
    TestTgs2600 tgs2600;

    float reference = 2500.0F;
    tgs2600.setReferenceResistance(reference);

    CHECK(tgs2600.sensorResistanceRelative( 500) == Approx(36.0f));
    CHECK(tgs2600.sensorResistanceRelative(1000) == Approx(16.0f));
    CHECK(tgs2600.sensorResistanceRelative(2000) == Approx( 6.0f));
    CHECK(tgs2600.sensorResistanceRelative(3000) == Approx( 2.6664f));
    CHECK(tgs2600.sensorResistanceRelative(4000) == Approx( 1.0f));
    CHECK(tgs2600.sensorResistanceRelative(5000) == Approx( 0.0f));
    CHECK(tgs2600.sensorResistanceRelative(8000) == Approx( 0.0f));
}

/*!
 * \brief Tests Sensors::Tgs2600::sensorResistance() and
 *        Sensors::Tgs2600::sensorResistanceCalibrated() on an ADC voltage drop.
 */
TEST_CASE("Tgs2600ADCVoltageDrop", "[tgs2600]")
{
    TestTgs2600 tgs2600;

    tgs2600.setReferenceHumidity(37);
    tgs2600.setReferenceTemperature(20);

    CHECK(tgs2600.sensorResistance(0) == UINT32_MAX);
    CHECK(tgs2600.sensorResistanceCalibrated(0) == UINT32_MAX);
}

/*!
 * \brief Tests mathematical limits of Sensors::Tgs2600::sensorResistance() and
 *        Sensors::Tgs2600::sensorResistanceCalibrated().
 */
TEST_CASE("Tgs2600Limits", "[tgs2600]")
{
    {
        Tgs2600<UINT32_MAX / 5000> tgs2600;
        CHECK(tgs2600.sensorResistance(0) == UINT32_MAX);
        CHECK(tgs2600.sensorResistance(1) == 4294106007U);
        CHECK(tgs2600.sensorResistance(4999) == 171U);
        CHECK(tgs2600.sensorResistance(5000) == 0U);
        CHECK(tgs2600.sensorResistance(UINT16_MAX) == 0U);
    }
    {
        Tgs2600<450> tgs2600;
        CHECK(tgs2600.sensorResistance(0) == UINT32_MAX);
        CHECK(tgs2600.sensorResistance(1) == 2249550U);
        CHECK(tgs2600.sensorResistance(4999) == 0U);
        CHECK(tgs2600.sensorResistance(5000) == 0U);
        CHECK(tgs2600.sensorResistance(UINT16_MAX) == 0U);
    }
}
