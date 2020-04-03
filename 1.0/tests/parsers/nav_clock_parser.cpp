/*
 * Copyright (C) 2019 GlobalLogic
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "GnssHalTesting"
#include <gtest/gtest.h>
#include <log/log.h>
#include <string>

#include "GnssNavClockParser.h"

// Dump UBX-NAV-CLOCK
//80 ee 2e 1d  489614976   ITow ms
//e8 74 06 00  423144      Clock bias ns
//d8 fd ff ff  -552        Clock drift ns/s
//d0 03 00 00  976         timeAccuracy ns
//43 0f 00 00  3907        Freq acc estimate ps/s

static const uint8_t ubxNavClockDump[] = {
    0x80, 0xee, 0x2e, 0x1d,
    0xe8, 0x74, 0x06, 0x00,
    0xd8, 0xfd, 0xff, 0xff,
    0xd0, 0x03, 0x00, 0x00,
    0x43, 0x0f, 0x00, 0x00 };

struct NavClockSample
{
    uint32_t iTow;
    int32_t clockBias;
    int32_t clockDrift;
    uint32_t timeAcc;
    uint32_t freqAccEst;
} navClockSample = {
    .iTow = 489614976,
    .clockBias = 423144,
    .clockDrift = -552,
    .timeAcc = 976,
    .freqAccEst = 3907,
};

class GnssNavClockParserTest : public GnssNavClockParser, public ::testing::Test {
protected:
    void SetUp() {}
};


TEST_F(GnssNavClockParserTest, createObjFromDump)
{
    GnssNavClockParser obj(ubxNavClockDump, (uint16_t)sizeof(ubxNavClockDump));
    (void)obj;
}

TEST_F(GnssNavClockParserTest, createObjFromDumpAndZeroLength)
{
    GnssNavClockParser obj(ubxNavClockDump, (uint16_t)0);
    (void)obj;
}

TEST_F(GnssNavClockParserTest, createObjFromNullPayload)
{
    GnssNavClockParser obj(nullptr, (uint16_t)sizeof(ubxNavClockDump));
    (void)obj;
}

TEST_F(GnssNavClockParserTest, createObjFromNullPayloadZeroLength)
{
    GnssNavClockParser obj(nullptr, (uint16_t)0);
    (void)obj;
}

TEST_F(GnssNavClockParserTest, createObjFromDumpMaxLength)
{
    GnssNavClockParser obj(ubxNavClockDump, (uint16_t)-1);
    (void)obj;
}

TEST_F(GnssNavClockParserTest, createObjFromNullPayloadMaxLength)
{
    GnssNavClockParser obj(nullptr, (uint16_t)-1);
    (void)obj;
}

TEST_F(GnssNavClockParserTest, createObjFromDumpRetrieveClockDone)
{
    GnssNavClockParser obj(ubxNavClockDump, (uint16_t)sizeof(ubxNavClockDump));
    MeasurementCb::GnssData data;
    ASSERT_EQ(ClockDone, obj.retrieveSvInfo(data));
}

TEST_F(GnssNavClockParserTest, createObjFromDumpAndZeroLengthRetrieveNotReady)
{
    GnssNavClockParser obj(ubxNavClockDump, (uint16_t)0);
    MeasurementCb::GnssData data;
    ASSERT_EQ(NotReady, obj.retrieveSvInfo(data));
}

TEST_F(GnssNavClockParserTest, createObjFromNullPayloadRetrieveNotReady)
{
    GnssNavClockParser obj(nullptr, (uint16_t)sizeof(ubxNavClockDump));
    MeasurementCb::GnssData data;
    ASSERT_EQ(NotReady, obj.retrieveSvInfo(data));
}

TEST_F(GnssNavClockParserTest, createObjFromNullPayloadZeroLengthRetrieveNotReady)
{
    GnssNavClockParser obj(nullptr, (uint16_t)0);
    MeasurementCb::GnssData data;
    ASSERT_EQ(NotReady, obj.retrieveSvInfo(data));
}

TEST_F(GnssNavClockParserTest, createObjFromDumpMaxLengthRetrieveNotReady)
{
    GnssNavClockParser obj(ubxNavClockDump, (uint16_t)-1);
    MeasurementCb::GnssData data;
    ASSERT_EQ(NotReady, obj.retrieveSvInfo(data));
}

TEST_F(GnssNavClockParserTest, createObjFromNullPayloadMaxLengthRetrieveNotReady)
{
    GnssNavClockParser obj(nullptr, (uint16_t)-1);
    MeasurementCb::GnssData data;
    ASSERT_EQ(NotReady, obj.retrieveSvInfo(data));
}

TEST_F(GnssNavClockParserTest, checkRetrievedDataFromDumpInput)
{
    GnssNavClockParser obj(ubxNavClockDump, sizeof(ubxNavClockDump));
    MeasurementCb::GnssData data;
    EXPECT_EQ(ClockDone, obj.retrieveSvInfo(data));
    MeasurementCb::GnssClock &clock = data.clock;
    EXPECT_EQ(navClockSample.clockBias, clock.biasNs);
    EXPECT_EQ(navClockSample.clockDrift, clock.driftNsps);
    EXPECT_EQ((uint32_t)0, clock.hwClockDiscontinuityCount);
}
