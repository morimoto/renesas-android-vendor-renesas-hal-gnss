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
#include <thread>

#include "GnssMeasQueue.h"
#include "GnssRxmMeasxParser.h"
#include "GnssNavClockParser.h"
#include "GnssNavTimeUTCParser.h"

static const size_t hundred = 100;
const uint8_t rxmMeasxMsg[] = {
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
    0x7a, 0x01, 0x4a, 0x02, 0x8b, 0xd7, 0x0b, 0x00, 0x00, 0x17, 0x00, 0x00
    };

static const uint32_t mStateFlags = MeasurementCb::GnssMeasurementState::STATE_TOW_DECODED |
        MeasurementCb::GnssMeasurementState::STATE_BIT_SYNC |
        MeasurementCb::GnssMeasurementState::STATE_SUBFRAME_SYNC;

TEST(GnssMeasQueueTest, pushOnePopOneExpectNotEmpty)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(true);
    auto sp = std::make_shared<GnssRxmMeasxParser> (rxmMeasxMsg, sizeof(rxmMeasxMsg));
    instance.push(sp);
    EXPECT_FALSE(instance.empty());
    auto rxm = instance.pop();
    EXPECT_TRUE(instance.empty());

    const uint32_t dumpedDataMeasCount = 6;
    MeasurementCb::GnssData data;

    ASSERT_EQ(GnssIParser::RxmDone, rxm->retrieveSvInfo(data));
    ASSERT_EQ(dumpedDataMeasCount, data.measurementCount);

    uint16_t exptectedSvid[] = {32, 2, 6, 17, 19, 24};
    double expectedCN0DbHz[] = {19.0, 17.0, 24.0, 15.0, 22.0, 29.0};
    const double prrScaling = 0.04;
    double expectedPseudoRangeRate[] = {16572.0, 21966.0, 18455.0, -6048.0, -905.0, 3147.0};

    for (uint32_t i = 0; i < data.measurementCount; ++i)
    {
        EXPECT_EQ(exptectedSvid[i], data.measurements[i].svid);
        EXPECT_EQ(CnstlType::GPS, data.measurements[i].constellation);
        EXPECT_EQ(mStateFlags, data.measurements[i].state);
        EXPECT_EQ(expectedCN0DbHz[i], data.measurements[i].cN0DbHz);
        EXPECT_EQ(MeasurementCb::GnssMultipathIndicator::INDICATOR_PRESENT, data.measurements[i].multipathIndicator);
        EXPECT_EQ((prrScaling *expectedPseudoRangeRate[i]), data.measurements[i].pseudorangeRateMps);
    }
}

TEST(GnssMeasQueueTest, pushTwoSameObjPopTwoExpectSizeCorrect)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(true);
    auto sp1 = std::make_shared<GnssRxmMeasxParser> (rxmMeasxMsg, sizeof(rxmMeasxMsg));
    auto sp2 = std::make_shared<GnssRxmMeasxParser> (rxmMeasxMsg, sizeof(rxmMeasxMsg));
    instance.push(sp1);
    EXPECT_FALSE(instance.empty());
    EXPECT_EQ((size_t)1, instance.getSize());

    instance.push(sp2);
    EXPECT_FALSE(instance.empty());
    EXPECT_EQ((size_t)2, instance.getSize());

    auto rxm1 = instance.pop();
    EXPECT_FALSE(instance.empty());
    EXPECT_EQ((size_t)1, instance.getSize());

    auto rxm2 = instance.pop();
    EXPECT_TRUE(instance.empty());
    EXPECT_EQ((size_t)0, instance.getSize());

    const uint32_t dumpedDataMeasCount = 6;
    MeasurementCb::GnssData data1;
    MeasurementCb::GnssData data2;

    ASSERT_EQ(GnssIParser::RxmDone, rxm1->retrieveSvInfo(data1));
    ASSERT_EQ(GnssIParser::RxmDone, rxm2->retrieveSvInfo(data2));
    ASSERT_EQ(dumpedDataMeasCount, data1.measurementCount);
    ASSERT_EQ(dumpedDataMeasCount, data2.measurementCount);

    uint16_t exptectedSvid[] = {32, 2, 6, 17, 19, 24};
    double expectedCN0DbHz[] = {19.0, 17.0, 24.0, 15.0, 22.0, 29.0};
    const double prrScaling = 0.04;
    double expectedPseudoRangeRate[] = {16572.0, 21966.0, 18455.0, -6048.0, -905.0, 3147.0};

    for (uint32_t i = 0; i < data2.measurementCount; ++i) {
        EXPECT_EQ(exptectedSvid[i], data2.measurements[i].svid);
        EXPECT_EQ(CnstlType::GPS, data2.measurements[i].constellation);
        EXPECT_EQ(mStateFlags, data2.measurements[i].state);
        EXPECT_EQ(expectedCN0DbHz[i], data2.measurements[i].cN0DbHz);
        EXPECT_EQ(MeasurementCb::GnssMultipathIndicator::INDICATOR_PRESENT, data2.measurements[i].multipathIndicator);
        EXPECT_EQ((prrScaling * expectedPseudoRangeRate[i]), data2.measurements[i].pseudorangeRateMps);
    }
}

TEST(GnssMeasQueueTest, pushHundredObjectsCheckSize)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(true);
    const size_t maxObjToPush = 100;

    for (size_t i = 1; i <= maxObjToPush; ++i) {
        auto sp = std::make_shared<GnssRxmMeasxParser> (rxmMeasxMsg, sizeof(rxmMeasxMsg));
        instance.push(sp);
        EXPECT_EQ(i, instance.getSize());
    }

    ASSERT_FALSE(instance.empty());
    ASSERT_EQ(maxObjToPush, instance.getSize());

    for (size_t i = maxObjToPush; i > 1; --i) {
        auto rxm = instance.pop();
        EXPECT_EQ((i - 1), instance.getSize());
        (void)rxm;
    }

    auto rxm = instance.pop();
    EXPECT_EQ((size_t)0, instance.getSize());
    EXPECT_TRUE(instance.empty());
    MeasurementCb::GnssData data;
    const uint32_t dumpedDataMeasCount = 6;

    ASSERT_EQ(GnssIParser::RxmDone, rxm->retrieveSvInfo(data));
    ASSERT_EQ(dumpedDataMeasCount, data.measurementCount);

    uint16_t exptectedSvid[] = {32, 2, 6, 17, 19, 24};
    double expectedCN0DbHz[] = {19.0, 17.0, 24.0, 15.0, 22.0, 29.0};
    const double prrScaling = 0.04;
    double expectedPseudoRangeRate[] = {16572.0, 21966.0, 18455.0, -6048.0, -905.0, 3147.0};

    for (uint32_t i = 0; i < data.measurementCount; ++i) {
        EXPECT_EQ(exptectedSvid[i], data.measurements[i].svid);
        EXPECT_EQ(CnstlType::GPS, data.measurements[i].constellation);
        EXPECT_EQ(mStateFlags, data.measurements[i].state);
        EXPECT_EQ(expectedCN0DbHz[i], data.measurements[i].cN0DbHz);
        EXPECT_EQ(MeasurementCb::GnssMultipathIndicator::INDICATOR_PRESENT, data.measurements[i].multipathIndicator);
        EXPECT_EQ((expectedPseudoRangeRate[i] * prrScaling), data.measurements[i].pseudorangeRateMps);
    }
}

TEST(GnssMeasQueueTest, popFromEmptyQueue)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(true);
    EXPECT_TRUE(instance.empty());
    EXPECT_EQ((size_t)0, instance.getSize());
    auto sp = instance.pop();
    EXPECT_EQ(nullptr, sp);
}

static void pushThread(void)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(true);
    for (size_t i = 0; i < hundred; ++i) {
        auto sp = std::make_shared<GnssRxmMeasxParser> (rxmMeasxMsg, sizeof(rxmMeasxMsg));
        instance.push(sp);
    }
}

static void popThread(void)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    size_t count = 0;
    uint16_t exptectedSvid[] = {32, 2, 6, 17, 19, 24};
    double expectedCN0DbHz[] = {19.0, 17.0, 24.0, 15.0, 22.0, 29.0};
    const double prrScaling = 0.04;
    double expectedPseudoRangeRate[] = {16572.0, 21966.0, 18455.0, -6048.0, -905.0, 3147.0};

    do {
        auto rxm = instance.pop();
        if (nullptr != rxm) {
            MeasurementCb::GnssData data;
            const uint32_t dumpedDataMeasCount = 6;

            ASSERT_EQ(GnssIParser::RxmDone, rxm->retrieveSvInfo(data));
            ASSERT_EQ(dumpedDataMeasCount, data.measurementCount);

            for (uint32_t i = 0; i < data.measurementCount; ++i) {
                EXPECT_EQ(exptectedSvid[i], data.measurements[i].svid);
                EXPECT_EQ(CnstlType::GPS, data.measurements[i].constellation);
                EXPECT_EQ(mStateFlags, data.measurements[i].state);
                EXPECT_EQ(expectedCN0DbHz[i], data.measurements[i].cN0DbHz);
                EXPECT_EQ(MeasurementCb::GnssMultipathIndicator::INDICATOR_PRESENT, data.measurements[i].multipathIndicator);
                EXPECT_EQ((prrScaling * expectedPseudoRangeRate[i]), data.measurements[i].pseudorangeRateMps);
            }
            ++count;
        }

    } while (count < hundred);
    EXPECT_EQ(count, hundred);
}

TEST(GnssMeasQueueTest, pushWaitPopFromTwoThreads)
{
    std::thread push(pushThread);
    push.join();

    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    ASSERT_EQ(hundred, instance.getSize());

    std::thread pop(popThread);
    pop.join();
    ASSERT_TRUE(instance.empty());
}

TEST(GnssMeasQueueTest, pushPopFromTwoThreads)
{
    std::thread push(pushThread);
    std::thread pop(popThread);
    push.join();
    pop.join();
}

TEST(GnssMeasQueueTest, setStateTrueCheckPushPop)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(true);
    auto sp = std::make_shared<GnssRxmMeasxParser> (rxmMeasxMsg, sizeof(rxmMeasxMsg));
    instance.push(sp);
    EXPECT_FALSE(instance.empty());

    auto rxm = instance.pop();
    EXPECT_TRUE(instance.empty());
    MeasurementCb::GnssData data;
    ASSERT_EQ(GnssIParser::RxmDone, rxm->retrieveSvInfo(data));
}

TEST(GnssMeasQueueTest, setStateFalseCheckPushPop)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(false);
    auto sp = std::make_shared<GnssRxmMeasxParser> (rxmMeasxMsg, sizeof(rxmMeasxMsg));
    instance.push(sp);
    EXPECT_TRUE(instance.empty());

    auto rxm = instance.pop();
    EXPECT_TRUE(instance.empty());
    ASSERT_EQ(nullptr, rxm);
}

TEST(GnssMeasQueueTest, setStateTruePushThenFalseCheckSize)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(true);
    auto sp = std::make_shared<GnssRxmMeasxParser> (rxmMeasxMsg, sizeof(rxmMeasxMsg));
    instance.push(sp);
    EXPECT_FALSE(instance.empty());

    instance.setState(false);
    EXPECT_EQ((size_t)0, instance.getSize());
    EXPECT_TRUE(instance.empty());
}

TEST(GnssMeasQueueTest, checkEmptyIfEmpty)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    EXPECT_TRUE(instance.empty());
}

TEST(GnssMeasQueueTest, checkEmptyIfNotEmpty)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(true);
    auto sp = std::make_shared<GnssRxmMeasxParser> (rxmMeasxMsg, sizeof(rxmMeasxMsg));
    instance.push(sp);
    EXPECT_FALSE(instance.empty());
}

TEST(GnssMeasQueueTest, checkPopIfStateFalse)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(false);
    auto rxm = instance.pop();
    EXPECT_TRUE(instance.empty());
    ASSERT_EQ(nullptr, rxm);

}

TEST(GnssMeasQueueTest, checkPushSetFalsePop)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(true);
    auto sp = std::make_shared<GnssRxmMeasxParser> (rxmMeasxMsg, sizeof(rxmMeasxMsg));
    instance.push(sp);
    EXPECT_FALSE(instance.empty());

    instance.setState(false);
    auto rxm = instance.pop();
    EXPECT_TRUE(instance.empty());
    ASSERT_EQ(nullptr, rxm);
}

TEST(GnssMeasQueueTest, checkPopIfEmptyAndStateOn)
{
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(true);
    EXPECT_TRUE(instance.empty());

    auto rxm = instance.pop();
    ASSERT_EQ(nullptr, rxm);

}

TEST(GnssMeasQueueTest, pushToOverflowQueue)
{
    const size_t ten_hundred = 1000;
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(true);
    auto sp = std::make_shared<GnssRxmMeasxParser> (rxmMeasxMsg, sizeof(rxmMeasxMsg));
    for (size_t i = 0; i < ten_hundred; ++i) {
        instance.push(sp);
        EXPECT_FALSE(instance.empty());
        EXPECT_TRUE(hundred >= instance.getSize());
    }
}

TEST(GnssMeasQueueTest, pushFromMultipleTheadsToOverflowQueue)
{
    std::thread push1(pushThread);
    std::thread push2(pushThread);
    std::thread push3(pushThread);
    std::thread push4(pushThread);
    push1.join();
    push2.join();
    push3.join();
    push4.join();

    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    EXPECT_FALSE(instance.empty());
    EXPECT_EQ(hundred, instance.getSize());
    instance.setState(false);
    EXPECT_TRUE(instance.empty());
}
