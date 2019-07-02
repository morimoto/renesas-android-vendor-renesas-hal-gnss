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

#include "GnssRxmMeasxParser.h"
#include <android/hardware/gnss/1.0/IGnss.h>


static const uint8_t rxmMeasxMsg[] = {
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

class GnssRxmMeasxParserTest : public GnssRxmMeasxParser, public ::testing::Test {
protected:
    void SetUp();

    uint16_t repeatedBlockSize;
    uint16_t singleBlockSize;
    uint32_t mStateFlags;
    uint32_t mStateKnownFlags;
};

void GnssRxmMeasxParserTest::SetUp()
{
    repeatedBlockSize = 24;
    singleBlockSize = 44;
    mStateFlags = IGnssMeasurementCallback::GnssMeasurementState::STATE_TOW_DECODED |
            IGnssMeasurementCallback::GnssMeasurementState::STATE_BIT_SYNC |
            IGnssMeasurementCallback::GnssMeasurementState::STATE_SUBFRAME_SYNC;

    mStateKnownFlags = IGnssMeasurementCallback::GnssMeasurementState::STATE_TOW_KNOWN |
            IGnssMeasurementCallback::GnssMeasurementState::STATE_BIT_SYNC |
            IGnssMeasurementCallback::GnssMeasurementState::STATE_SUBFRAME_SYNC;
}

TEST_F(GnssRxmMeasxParserTest, objectWithNullPtrPayload)
{
    //Test object creation without crashes with bad input parameters
    uint16_t len = GnssRxmMeasxParserTest::repeatedBlockSize + GnssRxmMeasxParserTest::singleBlockSize;
    GnssRxmMeasxParser obj(nullptr, len);
    (void)obj;
}

TEST_F(GnssRxmMeasxParserTest, objectWithNullPayloadShortLen)
{
    //Test object creation without crashes with bad input parameters
    uint16_t len = GnssRxmMeasxParserTest::repeatedBlockSize + GnssRxmMeasxParserTest::singleBlockSize - 1;
    GnssRxmMeasxParser obj(nullptr, len);
    (void)obj;
}

TEST_F(GnssRxmMeasxParserTest, objectWithNullPayloadZeroLen)
{
    //Test object creation without crashes with bad input parameters
    const uint16_t len = 0;
    GnssRxmMeasxParser obj(nullptr, len);
    (void)obj;
}

TEST_F(GnssRxmMeasxParserTest, objectWithZeroLenUbnormalPayload)
{
    //Test object creation without crashes with bad input paramete1rs
    const uint16_t len = 0;
    uint8_t payload[] = "This is my ubnormal payload";
    GnssRxmMeasxParser obj(payload, len);
    (void)obj;
}

TEST_F(GnssRxmMeasxParserTest, objectWithUbnormalPayload)
{
    //Test object creation without crashes with bad input parameters
    uint8_t payload[] = "This is my ubnormal payload";
    uint16_t len = (uint16_t)sizeof(payload);
    GnssRxmMeasxParser obj(payload, len);
    (void)obj;
}

TEST_F(GnssRxmMeasxParserTest, objectWithMaxLenUbnormalPayload)
{
    //Test object creation without crashes with bad input parameters
    const uint16_t len = (uint16_t)-1;
    uint8_t payload[] = "This is my ubnormal payload";
    GnssRxmMeasxParser obj(payload, len);
    (void)obj;
}

TEST_F(GnssRxmMeasxParserTest, objectWithMaxLenMaxPayload)
{
    //Test object creation without crashes with bad input parameters
    const uint16_t len = (uint16_t)-1;
    uint8_t payload[len] = {};
    GnssRxmMeasxParser obj(payload, len);
    (void)obj;
}

TEST_F(GnssRxmMeasxParserTest, objectSampleInputFrom)
{
    //Test object creation without crashes with bad input parameters
    const uint16_t len = (uint16_t)sizeof(rxmMeasxMsg);
    GnssRxmMeasxParser obj(rxmMeasxMsg, len);
    (void)obj;
}

TEST_F(GnssRxmMeasxParserTest, objectSampleIncompleteInput)
{
    //Test object creation without crashes with bad input parameters
    const uint16_t magic = 5;
    const uint16_t len = singleBlockSize + repeatedBlockSize + magic;
    GnssRxmMeasxParser obj(rxmMeasxMsg, len);
    (void)obj;
}

TEST_F(GnssRxmMeasxParserTest, objectSampleIncompleteInputNotFullBlocks)
{
    //Test object creation without crashes with bad input parameters
    const uint16_t magic = 5;
    const uint16_t len = singleBlockSize + repeatedBlockSize - magic;
    GnssRxmMeasxParser obj(rxmMeasxMsg, len);
    (void)obj;
}

TEST_F(GnssRxmMeasxParserTest, incompleteInputNotFullBlocksRetrieveNotReady)
{
    //Test object creation without crashes with bad input parameters
    const uint16_t magic = 5;
    const uint16_t len = singleBlockSize + repeatedBlockSize - magic;
    GnssRxmMeasxParser parser(rxmMeasxMsg, len);
    IGnssMeasurementCallback::GnssData data;
    EXPECT_TRUE(sizeof(rxmMeasxMsg) > (size_t)len);
    ASSERT_EQ(GnssIParser::NotReady, parser.retrieveSvInfo(data));
}

TEST_F(GnssRxmMeasxParserTest, testParseSingleBlockSampleInput)
{
    parseSingleBlock((uint8_t*)rxmMeasxMsg);
}

TEST_F(GnssRxmMeasxParserTest, testParseSingleBlockNullInput)
{
    parseSingleBlock(nullptr);
}

TEST_F(GnssRxmMeasxParserTest, retrieveDataNormalInput)
{
    const uint32_t dumpedDataMeasCount = 6;
    IGnssMeasurementCallback::GnssData data;
    GnssRxmMeasxParser parser(rxmMeasxMsg, sizeof(rxmMeasxMsg));

    ASSERT_EQ(GnssIParser::RxmDone, parser.retrieveSvInfo(data));
    ASSERT_EQ(dumpedDataMeasCount, data.measurementCount);

    uint16_t exptectedSvid[] = {32, 2, 6, 17, 19, 24};
    double expectedCN0DbHz[] = {19.0, 17.0, 24.0, 15.0, 22.0, 29.0};
    const double prrScaling = 0.04;
    double expectedPseudoRangeRate[] = {16572.0, 21966.0, 18455.0, -6048.0, -905.0, 3147.0};

    for(uint32_t i = 0; i < data.measurementCount; ++i)
    {
        EXPECT_EQ(exptectedSvid[i], data.measurements[i].svid);
        EXPECT_EQ(GnssConstellationType::GPS, data.measurements[i].constellation);
        EXPECT_EQ(mStateFlags, data.measurements[i].state);
        EXPECT_EQ(expectedCN0DbHz[i], data.measurements[i].cN0DbHz);
        EXPECT_EQ(IGnssMeasurementCallback::GnssMultipathIndicator::INDICATOR_PRESENT, data.measurements[i].multipathIndicator);
        EXPECT_EQ((expectedPseudoRangeRate[i]*prrScaling), data.measurements[i].pseudorangeRateMps);
    }
}

TEST_F(GnssRxmMeasxParserTest, retrieveDataIncompleteInput)
{
    IGnssMeasurementCallback::GnssData data;
    const uint16_t magic = 5;
    const uint16_t len = singleBlockSize + repeatedBlockSize + magic;

    const uint32_t dumpedDataMeasCount = 1;
    GnssRxmMeasxParser parser(rxmMeasxMsg, len);

    ASSERT_EQ(GnssIParser::RxmDone, parser.retrieveSvInfo(data));
    EXPECT_EQ(dumpedDataMeasCount, data.measurementCount);

    uint16_t exptectedSvid[] = {32, 2, 6, 17, 19, 24};
    double expectedCN0DbHz[] = {19.0, 17.0, 24.0, 15.0, 22.0, 29.0};
    const double prrScaling = 0.04;
    double expectedPseudoRangeRate[] = {16572.0, 21966.0, 18455.0, -6048.0, -905.0, 3147.0};

    for(uint32_t i = 0; i < data.measurementCount; ++i)
    {
        EXPECT_EQ(exptectedSvid[i], data.measurements[i].svid);
        EXPECT_EQ(GnssConstellationType::GPS, data.measurements[i].constellation);
        EXPECT_EQ(mStateFlags, data.measurements[i].state);
        EXPECT_EQ(expectedCN0DbHz[i], data.measurements[i].cN0DbHz);
        EXPECT_EQ(IGnssMeasurementCallback::GnssMultipathIndicator::INDICATOR_PRESENT, data.measurements[i].multipathIndicator);
        EXPECT_EQ((expectedPseudoRangeRate[i]*prrScaling), data.measurements[i].pseudorangeRateMps);
    }
}

TEST_F(GnssRxmMeasxParserTest, checkConstellationConvertion)
{
    const uint8_t GPS = 0;
    const uint8_t SBAS = 1;
    const uint8_t GALILEO = 2;
    const uint8_t BEIDOU = 3;
    const uint8_t QZSS = 5;
    const uint8_t GLONASS = 6;

    EXPECT_EQ(GnssConstellationType::GPS, getConstellationFromGnssId(GPS));
    EXPECT_EQ(GnssConstellationType::SBAS, getConstellationFromGnssId(SBAS));
    EXPECT_EQ(GnssConstellationType::GALILEO, getConstellationFromGnssId(GALILEO));
    EXPECT_EQ(GnssConstellationType::BEIDOU, getConstellationFromGnssId(BEIDOU));
    EXPECT_EQ(GnssConstellationType::QZSS, getConstellationFromGnssId(QZSS));
    EXPECT_EQ(GnssConstellationType::GLONASS, getConstellationFromGnssId(GLONASS));

    EXPECT_EQ(GnssConstellationType::UNKNOWN, getConstellationFromGnssId((uint8_t)-1));
    EXPECT_EQ(GnssConstellationType::UNKNOWN, getConstellationFromGnssId((uint8_t)7));
}

TEST_F(GnssRxmMeasxParserTest, checkNsConvertion)
{
    const uint32_t maxMs = UINT32_MAX;
    const uint32_t zeroMs = 0;
    const uint32_t oneMs = 1;
    const int64_t  msToNsMult = 1000000;
    int64_t expectedMaxNs = static_cast<int64_t>(maxMs) * msToNsMult;

    EXPECT_EQ((int64_t)0, scaleUp(zeroMs, msToNsMult));
    EXPECT_EQ(msToNsMult, scaleUp(oneMs, msToNsMult));
    EXPECT_EQ(expectedMaxNs, scaleUp(maxMs, msToNsMult));
    EXPECT_TRUE((int64_t)0 < scaleUp(maxMs, msToNsMult));
}

TEST_F(GnssRxmMeasxParserTest, checkTOWforGnssId)
{
    GnssRxmMeasxParser parser(rxmMeasxMsg, sizeof(rxmMeasxMsg));
    IGnssMeasurementCallback::GnssData data;
    ASSERT_EQ(GnssIParser::RxmDone, parser.retrieveSvInfo(data));
    EXPECT_EQ(mStateFlags, data.measurements[0].state);
    EXPECT_TRUE(data.measurements[0].receivedSvTimeInNs > 0);
    EXPECT_EQ(data.measurements[0].receivedSvTimeUncertaintyInNs, (int64_t)1);
}

TEST_F(GnssRxmMeasxParserTest, checkTOWforGnssIdTruncatedInput) {
    const uint8_t sampleInput[] = {
        0x01, 0x00, 0x00, 0x00, 0xc0, 0x9c, 0x03, 0x1d, 0xf0, 0x21, 0xa8, 0x1d, 0x10, 0x66, 0x03, 0x1d,
        0xc0, 0x9c, 0x03, 0x1d, 0xf4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x06, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x13, 0x01,
        0xbc, 0x40, 0x00, 0x00, 0x0a, 0x44, 0x00, 0x00, 0x14, 0x02, 0xc2, 0x01, 0xaf, 0xa7, 0x10, 0x00,
        0x00, 0x36, 0x00, 0x00,
    };

    GnssRxmMeasxParser parser(sampleInput, sizeof(sampleInput));
    IGnssMeasurementCallback::GnssData data;
    ASSERT_EQ(GnssIParser::RxmDone, parser.retrieveSvInfo(data));
    EXPECT_EQ(mStateFlags, data.measurements[0].state);
    EXPECT_TRUE(data.measurements[0].receivedSvTimeInNs > 0);
    EXPECT_EQ(data.measurements[0].receivedSvTimeUncertaintyInNs, (int64_t)1);
}

TEST_F(GnssRxmMeasxParserTest, checkTOWforAllGnssIdTruncatedInput)
{
    const uint32_t gnssIdOffset = singleBlockSize;
//    GPS = 0;
//    SBAS = 1;
//    GALILEO = 2;
//    BEIDOU = 3;
//    QZSS = 5;
//    GLONASS = 6;
    const uint8_t gnssIdArr [] = {0,1,2,3,4,5,6,7,8};

    uint8_t sampleInput[] = {
        0x01, 0x00, 0x00, 0x00, 0xc0, 0x9c, 0x03, 0x1d, 0xf0, 0x21, 0xa8, 0x1d, 0x10, 0x66, 0x03, 0x1d,
        0xc0, 0x9c, 0x03, 0x1d, 0xf4, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x01, 0x06, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x13, 0x01,
        0xbc, 0x40, 0x00, 0x00, 0x0a, 0x44, 0x00, 0x00, 0x14, 0x02, 0xc2, 0x01, 0xaf, 0xa7, 0x10, 0x00,
        0x00, 0x36, 0x00, 0x00,
    };

    for (size_t i = 0; i < sizeof(gnssIdArr); i++) {
        sampleInput[gnssIdOffset] = gnssIdArr[i];
        GnssRxmMeasxParser parser(sampleInput, sizeof(sampleInput));
        IGnssMeasurementCallback::GnssData data;
        ASSERT_EQ(GnssIParser::RxmDone, parser.retrieveSvInfo(data));

        switch(i) {
        case 1:
        case 2:
            EXPECT_EQ(mStateKnownFlags, data.measurements[0].state);
            EXPECT_TRUE(data.measurements[0].receivedSvTimeInNs > 0);
            EXPECT_TRUE(data.measurements[0].receivedSvTimeUncertaintyInNs > (int64_t)0);
            break;
        case 4:
            EXPECT_EQ(GnssConstellationType::UNKNOWN, data.measurements[0].constellation);
            EXPECT_EQ((uint32_t)IGnssMeasurementCallback::GnssMeasurementState::STATE_UNKNOWN,
                      data.measurements[0].state);
            EXPECT_TRUE(data.measurements[0].receivedSvTimeInNs == 0);
            EXPECT_TRUE(data.measurements[0].receivedSvTimeUncertaintyInNs == (int64_t)0);
            break;
        case 0:
        case 3:
        case 5:
        case 6:
            EXPECT_EQ(mStateFlags, data.measurements[0].state);
            EXPECT_TRUE(data.measurements[0].receivedSvTimeInNs > 0);
            EXPECT_TRUE(data.measurements[0].receivedSvTimeUncertaintyInNs > (int64_t)0);
            break;
        default:
            EXPECT_EQ((uint32_t)IGnssMeasurementCallback::GnssMeasurementState::STATE_UNKNOWN,
                      data.measurements[0].state);
            EXPECT_TRUE(data.measurements[0].receivedSvTimeInNs == 0);
            EXPECT_TRUE(data.measurements[0].receivedSvTimeUncertaintyInNs == (int64_t)0);
        }
    }
}

TEST_F(GnssRxmMeasxParserTest, testGetValidSvidForGnssIdPositive) {
    // Rnages of possible satellite vehicle id for each constellation
    const uint8_t gpsFirst = 1;
    const uint8_t gpsLast = 32;

    const uint8_t sbasOneFirst = 120;
    const uint8_t sbasOneLast = 151;
    const uint8_t sbasTwoFirst = 183;
    const uint8_t sbasTwoLast = 192;

    const uint8_t galileoFirst = 1;
    const uint8_t galileoLast = 36;

    const uint8_t qzssFirst = 193;
    const uint8_t qzssLast = 200;

    const uint8_t bdFirst = 1;
    const uint8_t bdLast = 37;

    const uint8_t glonassFcnFirst = 93;
    const uint8_t glonassFcnLast = 106;
    const uint8_t glonassFirst = 1;
    const uint8_t glonassLast = 24;

    EXPECT_EQ(gpsFirst, getValidSvidForGnssId(UbxGnssId::GPS, gpsFirst));
    EXPECT_EQ(gpsLast, getValidSvidForGnssId(UbxGnssId::GPS, gpsLast));
    EXPECT_EQ((gpsFirst + 1), getValidSvidForGnssId(UbxGnssId::GPS, gpsFirst + 1));

    EXPECT_EQ(sbasOneFirst, getValidSvidForGnssId(UbxGnssId::SBAS, sbasOneFirst));
    EXPECT_EQ(sbasOneLast, getValidSvidForGnssId(UbxGnssId::SBAS, sbasOneLast));
    EXPECT_EQ((sbasOneFirst + 1), getValidSvidForGnssId(UbxGnssId::SBAS, sbasOneFirst + 1));
    EXPECT_EQ(sbasTwoFirst, getValidSvidForGnssId(UbxGnssId::SBAS, sbasTwoFirst));
    EXPECT_EQ(sbasTwoLast, getValidSvidForGnssId(UbxGnssId::SBAS, sbasTwoLast));
    EXPECT_EQ((sbasTwoFirst + 1), getValidSvidForGnssId(UbxGnssId::SBAS, sbasTwoFirst + 1));

    EXPECT_EQ(galileoFirst, getValidSvidForGnssId(UbxGnssId::GALILEO, galileoFirst));
    EXPECT_EQ(galileoLast, getValidSvidForGnssId(UbxGnssId::GALILEO, galileoLast));
    EXPECT_EQ(galileoFirst + 1, getValidSvidForGnssId(UbxGnssId::GALILEO, galileoFirst + 1));

    EXPECT_EQ(qzssFirst, getValidSvidForGnssId(UbxGnssId::QZSS, qzssFirst));
    EXPECT_EQ(qzssLast, getValidSvidForGnssId(UbxGnssId::QZSS, qzssLast));
    EXPECT_EQ(qzssFirst + 1, getValidSvidForGnssId(UbxGnssId::QZSS, qzssFirst + 1));

    EXPECT_EQ(bdFirst, getValidSvidForGnssId(UbxGnssId::BEIDOU, bdFirst));
    EXPECT_EQ(bdLast, getValidSvidForGnssId(UbxGnssId::BEIDOU, bdLast));
    EXPECT_EQ(bdFirst + 1, getValidSvidForGnssId(UbxGnssId::BEIDOU, bdFirst + 1));

    EXPECT_EQ(glonassFirst, getValidSvidForGnssId(UbxGnssId::GLONASS, glonassFirst));
    EXPECT_EQ(glonassLast, getValidSvidForGnssId(UbxGnssId::GLONASS, glonassLast));
    EXPECT_EQ(glonassFirst + 1, getValidSvidForGnssId(UbxGnssId::GLONASS, glonassFirst + 1));
    EXPECT_EQ(glonassFcnFirst, getValidSvidForGnssId(UbxGnssId::GLONASS, glonassFcnFirst));
    EXPECT_EQ(glonassFcnLast, getValidSvidForGnssId(UbxGnssId::GLONASS, glonassFcnLast));
    EXPECT_EQ(glonassFcnFirst + 1, getValidSvidForGnssId(UbxGnssId::GLONASS, glonassFcnFirst + 1));
}

TEST_F(GnssRxmMeasxParserTest, testGetValidSvidForGnssIdNegative) {
    // Ranges of possible satellite vehicle id for each constellation
    const uint8_t gpsFirst = 1;
    const uint8_t gpsLast = 32;

    const uint8_t sbasOneFirst = 120;
    const uint8_t sbasOneLast = 151;
    const uint8_t sbasTwoFirst = 183;
    const uint8_t sbasTwoLast = 192;

    const uint8_t galileoFirst = 1;
    const uint8_t galileoLast = 36;

    const uint8_t qzssFirst = 193;
    const uint8_t qzssLast = 200;

    const uint8_t bdFirst = 1;
    const uint8_t bdLast = 37;

    const uint8_t glonassFcnFirst = 93;
    const uint8_t glonassFirst = 1;
    const uint8_t glonassLast = 24;

    EXPECT_EQ(gpsFirst, getValidSvidForGnssId(UbxGnssId::GPS, gpsFirst - 1));
    EXPECT_EQ(gpsFirst, getValidSvidForGnssId(UbxGnssId::GPS, gpsLast + 1));

    EXPECT_EQ(sbasTwoFirst, getValidSvidForGnssId(UbxGnssId::SBAS, sbasOneFirst - 1));
    EXPECT_EQ(sbasTwoFirst, getValidSvidForGnssId(UbxGnssId::SBAS, sbasOneLast + 1));

    EXPECT_EQ(sbasTwoFirst, getValidSvidForGnssId(UbxGnssId::SBAS, sbasTwoFirst - 1));
    EXPECT_EQ(sbasTwoFirst, getValidSvidForGnssId(UbxGnssId::SBAS, sbasTwoLast + 1));

    EXPECT_EQ(galileoFirst, getValidSvidForGnssId(UbxGnssId::GALILEO, galileoFirst - 1));
    EXPECT_EQ(galileoFirst, getValidSvidForGnssId(UbxGnssId::GALILEO, galileoLast + 1));

    EXPECT_EQ(qzssFirst, getValidSvidForGnssId(UbxGnssId::QZSS, qzssFirst - 1));
    EXPECT_EQ(qzssFirst, getValidSvidForGnssId(UbxGnssId::QZSS, qzssLast + 1));

    EXPECT_EQ(bdFirst, getValidSvidForGnssId(UbxGnssId::BEIDOU, bdFirst - 1));
    EXPECT_EQ(bdFirst, getValidSvidForGnssId(UbxGnssId::BEIDOU, bdLast + 1));

    EXPECT_EQ(glonassFcnFirst, getValidSvidForGnssId(UbxGnssId::GLONASS, glonassFirst - 1));
    EXPECT_EQ(glonassFcnFirst, getValidSvidForGnssId(UbxGnssId::GLONASS, glonassLast + 1));
    EXPECT_EQ(glonassFcnFirst, getValidSvidForGnssId(UbxGnssId::GLONASS, 0xff));
}
