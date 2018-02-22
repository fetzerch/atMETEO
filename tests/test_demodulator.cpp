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
 * \brief Unit tests for \ref libsensors_demodulator.
 */

#include <catch.hpp>

#include "lib/utils.h"
#include "lib/demodulator.h"

using ::Sensors::Demodulator;
using ::Sensors::DemodulatorStatus;
using ::Sensors::BiphaseMark;

/*!
 * \brief Tests Sensors::Demodulator with Sensors::BiphaseMark configuration.
 */
TEST_CASE("DemodulatingWithBiphaseMarkConfiguration", "[demodulator]")
{
    Demodulator<BiphaseMark<200, 675, 675, 1150>> bmc;
    DemodulatorStatus status;

    SECTION("InRange Values") {
        CHECK(bmc.addPulseWidth(900) == DemodulatorStatus::Complete);
        CHECK(bmc.addPulseWidth(450) == DemodulatorStatus::Incomplete);
        CHECK(bmc.addPulseWidth(450) == DemodulatorStatus::Complete);
    }

    SECTION("OutOfRange Values") {
        CHECK(bmc.addPulseWidth(199) == DemodulatorStatus::OutOfRangeError);
        CHECK(bmc.addPulseWidth(1151) == DemodulatorStatus::OutOfRangeError);
    }
}
