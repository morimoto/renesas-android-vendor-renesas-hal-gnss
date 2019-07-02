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

#include "GnssParserCommonImpl.h"

static const uint8_t sample[] = {
    0x01, 0x02, 0x03, 0x04,
    0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x00, 0x00};


class GnssParserCommonImplTest : public GnssParserCommonImpl, public ::testing::Test {
protected:
    void SetUp() {}
    uint8_t retrieveSvInfo(IGnssMeasurementCallback::GnssData &gnssData) final;
};

uint8_t GnssParserCommonImplTest::retrieveSvInfo(IGnssMeasurementCallback::GnssData &gnssData)
{
    (void)gnssData;
    return 0;
}

TEST_F(GnssParserCommonImplTest, getUint16NormalInput)
{
    const uint16_t expected = 513;
    uint16_t res = GnssParserCommonImpl::getValue<uint16_t>(sample);
    ASSERT_EQ(expected, res);
}

TEST_F(GnssParserCommonImplTest, getUint16ZeroBytes)
{
    const uint16_t expected = 0;
    uint16_t res = GnssParserCommonImpl::getValue<uint16_t>(&sample[sizeof(sample) - 2]);
    ASSERT_EQ(expected, res);
}

TEST_F(GnssParserCommonImplTest, getUint16FFBytes)
{
    const uint16_t expected = (uint16_t)-1;
    uint16_t res = GnssParserCommonImpl::getValue<uint16_t>(&sample[sizeof(sample) - 4]);
    ASSERT_EQ(expected, res);
}

TEST_F(GnssParserCommonImplTest, getUint16NullInput)
{
    const uint16_t expected = 0;
    uint16_t res = GnssParserCommonImpl::getValue<uint16_t>(nullptr);
    ASSERT_EQ(expected, res);
}

TEST_F(GnssParserCommonImplTest, getUint32NormalInput)
{
    const uint32_t expected = 67305985;
    uint32_t res = GnssParserCommonImpl::getValue<uint32_t>(sample);
    ASSERT_EQ(expected, res);
}

TEST_F(GnssParserCommonImplTest, getUint32ZeroBytes)
{
    const uint32_t expected = 0;
    uint32_t res = GnssParserCommonImpl::getValue<uint32_t>(&sample[4]);
    ASSERT_EQ(expected, res);
}

TEST_F(GnssParserCommonImplTest, getUint32FFBytes)
{
    const uint32_t expected = (uint32_t) -1;
    uint32_t res = GnssParserCommonImpl::getValue<uint32_t>(&sample[8]);
    ASSERT_EQ(expected, res);
}

TEST_F(GnssParserCommonImplTest, getUint32NullInput)
{
    const uint32_t expected = 0;
    uint32_t res = GnssParserCommonImpl::getValue<uint32_t>(nullptr);
    ASSERT_EQ(expected, res);
}

TEST_F(GnssParserCommonImplTest, scaleUpIntDouble)
{
    const int varInt = 10;
    const double varDouble = 0.25;
    const double expDouble = 2.5;
    auto res = scaleUp(varInt, varDouble);
    ASSERT_EQ(expDouble, res);
    ASSERT_EQ(sizeof(res), sizeof(expDouble));
}

TEST_F(GnssParserCommonImplTest, scaleUpDoubleUint32)
{
    const uint32_t varUint32 = 10;
    const double varDouble = 2.25;
    const uint32_t expUint32 = 20;
    auto res = scaleUp(varDouble, varUint32);
    ASSERT_EQ(expUint32, res);
    ASSERT_EQ(sizeof(res), sizeof(varUint32));
}

TEST_F(GnssParserCommonImplTest, scaleUpFloatUint16)
{
    const uint16_t varInt = 10;
    const float varDouble = 0.25;
    const float expFloat = 2.5;
    auto res = scaleUp(varInt, varDouble);
    ASSERT_EQ(expFloat, res);
    ASSERT_EQ(sizeof(res), sizeof(expFloat));
}

TEST_F(GnssParserCommonImplTest, scaleUpUint16Float)
{
    const uint16_t varUint16 = 10;
    const float varFloat = 0.25;
    const float expFloat = 2.5;
    auto res = scaleUp(varUint16, varFloat);
    ASSERT_EQ(expFloat, res);
    ASSERT_EQ(sizeof(res), sizeof(varFloat));
}

TEST_F(GnssParserCommonImplTest, scaleDownIntDouble)
{
    const int varInt = 10;
    const double varDouble = 0.25;
    const double expDouble = 40.0;
    auto res = scaleDown(varInt, varDouble);
    ASSERT_EQ(expDouble, res);
    ASSERT_EQ(sizeof(res), sizeof(expDouble));
}

TEST_F(GnssParserCommonImplTest, scaleDownDoubleUint32)
{
    const uint32_t varUint32 = 10;
    const double varDouble = 0.25;
    const uint32_t expUint32 = 0;
    auto res = scaleDown(varDouble, varUint32);
    ASSERT_EQ(expUint32, res);
    ASSERT_EQ(sizeof(res), sizeof(varUint32));
}

TEST_F(GnssParserCommonImplTest, scaleDownFloatUint16)
{
    const uint16_t varUint16 = 10;
    const float varFloat = 0.25;
    const uint16_t expUint16 = 0;
    auto res = scaleDown(varFloat, varUint16);
    ASSERT_EQ(expUint16, res);
    ASSERT_EQ(sizeof(res), sizeof(expUint16));
}

TEST_F(GnssParserCommonImplTest, scaleDownUint16Float)
{
    const uint16_t varUint16 = 10;
    const float varFloat = 0.25;
    const float expFloat = 40.0;
    auto res = scaleDown(varUint16, varFloat);
    ASSERT_EQ(expFloat, res);
    ASSERT_EQ(sizeof(res), sizeof(varFloat));
}

TEST_F(GnssParserCommonImplTest, hexdumpNormalInput)
{
    //expected result is no segfault
    GnssParserCommonImpl::hexdump("/data/app/testHexdump", (void*)sample, sizeof(sample));
}

TEST_F(GnssParserCommonImplTest, hexdumpFilenameNull)
{
    //expected result is no segfault
    GnssParserCommonImpl::hexdump(nullptr, (void*)sample, sizeof(sample));
}

TEST_F(GnssParserCommonImplTest, hexdumpSourceNull)
{
    //expected result is no segfault
    GnssParserCommonImpl::hexdump("/data/app/testHexdump", nullptr, sizeof(sample));
}


TEST_F(GnssParserCommonImplTest, hexdumpZeroLength)
{
    //expected result is no segfault
    GnssParserCommonImpl::hexdump("/data/app/testHexdump", (void*)sample, 0);
}

TEST_F(GnssParserCommonImplTest, testInRangePositive)
{
    const int reference_value = 10;
    int value = 10;

    inRange(0, 20, value);
    EXPECT_EQ(reference_value, value);

    inRange(10, 30, value);
    EXPECT_EQ(reference_value, value);

    inRange(0, 10, value);
    EXPECT_EQ(reference_value, value);

    inRange(10, 10, value);
    EXPECT_EQ(reference_value, value);

    inRange(0, 10000, value);
    EXPECT_EQ(reference_value, value);
}

TEST_F(GnssParserCommonImplTest, testInRangeNegative)
{
    const int reference_value = 10;
    const int beginRange = 10;

    int value = 0;
    inRange(beginRange, 40, value);
    EXPECT_EQ(reference_value, value);

    value = 50;
    inRange(beginRange, 40, value);
    EXPECT_EQ(reference_value, value);
}

TEST_F(GnssParserCommonImplTest, testInRangesPositive)
{
    const int reference_value = 10;
    int value = 10;

    inRanges(0, 20, 30, 40, value);
    EXPECT_EQ(reference_value, value);

    inRanges(0, 10, 30, 40, value);
    EXPECT_EQ(reference_value, value);

    inRanges(10, 20, 30, 40, value);
    EXPECT_EQ(reference_value, value);

    inRanges(30, 42, 0, 20, value);
    EXPECT_EQ(reference_value, value);

    inRanges(30, 42, 0, 10, value);
    EXPECT_EQ(reference_value, value);

    inRanges(30, 42, 10, 10, value);
    EXPECT_EQ(reference_value, value);

    inRanges(30, 42, 10, 11, value);
    EXPECT_EQ(reference_value, value);
}

TEST_F(GnssParserCommonImplTest, testInRangesNegative)
{
    int value = 10;
    inRanges(1, 9, 11, 40, value);
    EXPECT_EQ(11, value);

    value = 41;
    inRanges(1, 9, 11, 40, value);
    EXPECT_EQ(11, value);

    value = 0;
    inRanges(1, 9, 11, 40, value);
    EXPECT_EQ(11, value);
}
