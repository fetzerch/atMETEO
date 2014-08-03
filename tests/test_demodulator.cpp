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

#include <gmock/gmock.h>

#include "lib/utils.h"
#include "lib/demodulator.h"

using ::testing::ElementsAre;
using ::Sensors::Demodulator;
using ::Sensors::DemodulatorStatus;
using ::Sensors::BiphaseMark;

/*!
 * \brief Tests Sensors::Demodulator with Sensors::BiphaseMark configuration.
 */
TEST(DemodulatorTest, BMCDecoder)
{
    Demodulator<BiphaseMark<200, 675, 675, 1150>> bmcDecoder;
    DemodulatorStatus status;

    status = bmcDecoder.addPulseWidth(199);
    EXPECT_EQ(DemodulatorStatus::OutOfRangeError, status);

    status = bmcDecoder.addPulseWidth(1151);
    EXPECT_EQ(DemodulatorStatus::OutOfRangeError, status);

    bmcDecoder.reset();

    status = bmcDecoder.addPulseWidth(900);
    EXPECT_EQ(DemodulatorStatus::Complete, status);

    status = bmcDecoder.addPulseWidth(450);
    EXPECT_EQ(DemodulatorStatus::Incomplete, status);

    status = bmcDecoder.addPulseWidth(450);
    EXPECT_EQ(DemodulatorStatus::Complete, status);
}
