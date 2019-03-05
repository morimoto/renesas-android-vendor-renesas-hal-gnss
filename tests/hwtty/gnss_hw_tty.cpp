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
#include <android/hardware/gnss/1.0/IGnss.h>
#include <stdio.h>

#include "GnssHw.h"
#include "GnssIParser.h"
#include "GnssParserCommonImpl.h"
#include "GnssMeasurement.h"
#include "GnssRxmMeasxParser.h"
#include "GnssMeasQueue.h"

using namespace android::hardware::gnss::V1_0::renesas;

const char rxmMeasxMsg[] = {
    0x01, 0x00, 0x00, 0x00, 0xc0, 0x9c, 0x03, 0x1d, 0xf0, 0x21, 0xa8, 0x1d, 0x10, 0x66, 0x03, 0x1d,
    0xc0, 0x9c, 0x03, 0x1d, 0xf4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x06, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x13, 0x01,
    0xbc, 0x40, 0x00, 0x00, 0x0a, 0x44, 0x00, 0x00, 0x14, 0x02, 0xc2, 0x01, 0xaf, 0xa7, 0x10, 0x00,
    0x00, 0x36, 0x00, 0x00, 0x00, 0x02, 0x11, 0x01, 0xce, 0x55, 0x00, 0x00, 0x2e, 0x5a, 0x00, 0x00,
    0xa7, 0x03, 0x71, 0x01, 0x33, 0x42, 0x1d, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 0x06, 0x18, 0x01,
    0x17, 0x48, 0x00, 0x00, 0xc5, 0x4b, 0x00, 0x00, 0x58, 0x03, 0xc5, 0x00, 0x3d, 0xc8, 0x1a, 0x00,
    0x00, 0x17, 0x00, 0x00, 0x00, 0x11, 0x0f, 0x01, 0x60, 0xe8, 0xff, 0xff, 0x2c, 0xe7, 0xff, 0xff,
    0xa7, 0x01, 0xb9, 0x00, 0xc3, 0x3c, 0x0d, 0x00, 0x00, 0x2e, 0x00, 0x00, 0x00, 0x13, 0x16, 0x01,
    0x77, 0xfc, 0xff, 0xff, 0x49, 0xfc, 0xff, 0xff, 0x35, 0x01, 0xba, 0x00, 0xe1, 0xab, 0x09, 0x00,
    0x00, 0x31, 0x00, 0x00, 0x00, 0x18, 0x1d, 0x01, 0x4b, 0x0c, 0x00, 0x00, 0xeb, 0x0c, 0x00, 0x00,
    0x7a, 0x01, 0x4a, 0x02, 0x8b, 0xd7, 0x0b, 0x00, 0x00, 0x17, 0x00, 0x00};

class GnssHwTTYTest : public GnssHwTTY, public ::testing::Test {
protected:
    void SetUp();

    uint16_t repeatedBlockSize;
    uint16_t singleBlockSize;
    uint8_t cl;
    uint8_t id;
};

void GnssHwTTYTest::SetUp()
{
    repeatedBlockSize = 24;
    singleBlockSize = 44;
    cl = 0x02;
    id = 0x14;
}

TEST_F(GnssHwTTYTest, selectParserRxmMeasxDumpNormalInput)
{
    GnssMeasQueue &instance = GnssMeasQueue::getInstance();
    instance.setState(true);

    uint16_t payloadLen = (uint16_t)sizeof(rxmMeasxMsg);
    SelectParser(cl, id, rxmMeasxMsg, payloadLen);

    EXPECT_FALSE(instance.empty());

    const uint32_t emptyDataMeasCount = 0;
    const uint32_t dumpedDataMeasCount = 6;

    IGnssMeasurementCallback::GnssData data;
    data.measurementCount = 0;
    EXPECT_EQ(emptyDataMeasCount, data.measurementCount);

    auto parser = instance.pop();
    ASSERT_EQ(GnssIParser::RxmDone, parser->retrieveSvInfo(data));
    ASSERT_EQ(dumpedDataMeasCount, data.measurementCount);

    uint16_t exptectedSvid[] = {32, 2, 6, 17, 19, 24};
    double expectedCN0DbHz[] = {19.0, 17.0, 24.0, 15.0, 22.0, 29.0};
    const double prrScaling = 0.04;
    double expectedPseudoRangeRate[] = {16572.0, 21966.0, 18455.0, -6048.0, -905.0, 3147.0};

    for (uint32_t i = 0; i < data.measurementCount; ++i) {
        EXPECT_EQ(exptectedSvid[i], data.measurements[i].svid);
        EXPECT_EQ(GnssConstellationType::GPS, data.measurements[i].constellation);
        EXPECT_EQ((uint32_t)IGnssMeasurementCallback::GnssMeasurementState::STATE_TOW_DECODED, data.measurements[i].state);
        EXPECT_EQ(expectedCN0DbHz[i], data.measurements[i].cN0DbHz);
        EXPECT_EQ(IGnssMeasurementCallback::GnssMultipathIndicator::INDICATOR_PRESENT, data.measurements[i].multipathIndicator);
        EXPECT_EQ((expectedPseudoRangeRate[i]*prrScaling), data.measurements[i].pseudorangeRateMps);
    }
}

//TEST_F(GnssHwTTYTest, selectParserRxmGnssMeasurementsCallbackThreadNormal)
TEST_F(GnssHwTTYTest, DISABLED_selectParserRxmGnssMeasurementsCallbackThreadNormal)
{
    GnssMeasQueue &instance = GnssMeasQueue::getInstance();
    instance.setState(true);

    uint16_t payloadLen = (uint16_t)sizeof(rxmMeasxMsg);
    SelectParser(cl, id, rxmMeasxMsg, payloadLen);

    EXPECT_FALSE(instance.empty());

    const uint32_t emptyDataMeasCount = 0;
    const uint32_t dumpedDataMeasCount = 6;

    IGnssMeasurementCallback::GnssData data;
    data.measurementCount = 0;
    EXPECT_EQ(emptyDataMeasCount, data.measurementCount);

    auto parser = instance.pop();
    ASSERT_EQ(GnssIParser::RxmDone, parser->retrieveSvInfo(data));
    ASSERT_EQ(dumpedDataMeasCount, data.measurementCount);

    uint16_t exptectedSvid[] = {32, 2, 6, 17, 19, 24};
    double expectedCN0DbHz[] = {19.0, 17.0, 24.0, 15.0, 22.0, 29.0};
    double expectedPseudoRangeRate[] = {16572.0, 21966.0, 18455.0, -6048.0, -905.0, 3147.0};

    for (uint32_t i = 0; i < data.measurementCount; ++i) {
        EXPECT_EQ(exptectedSvid[i], data.measurements[i].svid);
        EXPECT_EQ(GnssConstellationType::GPS, data.measurements[i].constellation);
        EXPECT_EQ((uint32_t)IGnssMeasurementCallback::GnssMeasurementState::STATE_TOW_DECODED, data.measurements[i].state);
        EXPECT_EQ(expectedCN0DbHz[i], data.measurements[i].cN0DbHz);
        EXPECT_EQ(IGnssMeasurementCallback::GnssMultipathIndicator::INDICATOR_PRESENT, data.measurements[i].multipathIndicator);
        EXPECT_EQ(expectedPseudoRangeRate[i], data.measurements[i].pseudorangeRateMps);
    }

    const size_t five = 5;
    for (size_t i = 0; i < five; ++i) {
        SelectParser(cl, id, rxmMeasxMsg, payloadLen);
    }

    GnssMeasurement gnssMeas;
    gnssMeas.callbackThread();
}

//TEST_F(GnssHwTTYTest, checkCallbackThreadWithOneMsgInQueue)
TEST_F(GnssHwTTYTest, DISABLED_checkCallbackThreadWithOneMsgInQueue)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(true);

    uint16_t payloadLen = (uint16_t)sizeof(rxmMeasxMsg);
    SelectParser(cl, id, rxmMeasxMsg, payloadLen);

    ASSERT_EQ((size_t)1, instance.getSize());

    GnssMeasurement gnssMeas;
    gnssMeas.callbackThread();
}


//TEST_F(GnssHwTTYTest, checkCallbackThreadWithFiveMsgInQueue)
TEST_F(GnssHwTTYTest, DISABLED_checkCallbackThreadWithFiveMsgInQueue)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(true);

    uint16_t payloadLen = (uint16_t)sizeof(rxmMeasxMsg);
    const size_t five = 5;
    for (size_t i = 0; i < five; ++i) {
        SelectParser(cl, id, rxmMeasxMsg, payloadLen);
    }

    ASSERT_EQ(five, instance.getSize());

    GnssMeasurement gnssMeas;
    gnssMeas.callbackThread();
}

//TEST_F(GnssHwTTYTest, checkCallbackThreadWithTenMsgInQueue)
TEST_F(GnssHwTTYTest, DISABLED_checkCallbackThreadWithTenMsgInQueue)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(true);

    uint16_t payloadLen = (uint16_t)sizeof(rxmMeasxMsg);
    const size_t ten = 10;
    for (size_t i = 0; i < ten; ++i) {
        SelectParser(cl, id, rxmMeasxMsg, payloadLen);
    }

    ASSERT_EQ(ten, instance.getSize());

    GnssMeasurement gnssMeas;
    gnssMeas.callbackThread();
}
