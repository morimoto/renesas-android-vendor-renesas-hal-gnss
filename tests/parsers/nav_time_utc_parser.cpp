#define LOG_TAG "GnssHalTesting"
#include <gtest/gtest.h>
#include <log/log.h>
#include <string>

#include "GnssNavTimeUTCParser.h"
#include "GnssMeasQueue.h"

static const uint8_t navTimeUtcDump[] = {
    0x60, 0xee, 0x5e, 0x0c,
    0x0a, 0x00, 0x00, 0x00,
    0x31, 0xde, 0xfb, 0xff,
    0xe3, 0x07, 0x02, 0x1a,
    0x09, 0x26, 0x32, 0x37,
};

class GnssNavTimeUTCParserTest : public GnssNavTimeUTCParser, public ::testing::Test {
 protected:
    void SetUp() {}
};

TEST_F(GnssNavTimeUTCParserTest, createObjDumpInput)
{
    GnssNavTimeUTCParser obj(navTimeUtcDump, (uint16_t)sizeof(navTimeUtcDump));
    (void) obj;
}

TEST_F(GnssNavTimeUTCParserTest, createObjWithNullPayloadNormalSize)
{
    GnssNavTimeUTCParser obj(nullptr, (uint16_t)sizeof(navTimeUtcDump));
    (void) obj;
}

TEST_F(GnssNavTimeUTCParserTest, createObjWithDumpInputZeroLen)
{
    GnssNavTimeUTCParser obj(navTimeUtcDump, (uint16_t)0);
    (void) obj;
}

TEST_F(GnssNavTimeUTCParserTest, createObjWithDumpInputMaxLen)
{
    GnssNavTimeUTCParser obj(navTimeUtcDump, (uint16_t)-1);
    (void) obj;
}

TEST_F(GnssNavTimeUTCParserTest, createObjWithNullInputZeroLen)
{
    GnssNavTimeUTCParser obj(nullptr, (uint16_t)0);
    (void) obj;
}

TEST_F(GnssNavTimeUTCParserTest, createObjWithNullInputMaxLen)
{
    GnssNavTimeUTCParser obj(nullptr, (uint16_t)-1);
    (void) obj;
}

TEST_F(GnssNavTimeUTCParserTest, checkRetrieveNotReadyNullPayloadNormalSize)
{
    GnssNavTimeUTCParser obj(nullptr, (uint16_t)sizeof(navTimeUtcDump));
    MeasurementCb::GnssData data;
    ASSERT_EQ(NotReady, obj.retrieveSvInfo(data));
}

TEST_F(GnssNavTimeUTCParserTest, checkRetrieveNotReadyWithDumpInputZeroLen)
{
    GnssNavTimeUTCParser obj(navTimeUtcDump, (uint16_t)0);
    MeasurementCb::GnssData data;
    ASSERT_EQ(NotReady, obj.retrieveSvInfo(data));
}

TEST_F(GnssNavTimeUTCParserTest, checkRetrieveNotReadyWithDumpInputMaxLen)
{
    GnssNavTimeUTCParser obj(navTimeUtcDump, (uint16_t)-1);
    MeasurementCb::GnssData data;
    ASSERT_EQ(NotReady, obj.retrieveSvInfo(data));
}

TEST_F(GnssNavTimeUTCParserTest, checkRetrieveNotReadyWithNullInputZeroLen)
{
    GnssNavTimeUTCParser obj(nullptr, (uint16_t)0);
    MeasurementCb::GnssData data;
    ASSERT_EQ(NotReady, obj.retrieveSvInfo(data));
}

TEST_F(GnssNavTimeUTCParserTest, checkRetrieveNotReadyWithNullInputMaxLen)
{
    GnssNavTimeUTCParser obj(nullptr, (uint16_t)-1);
    MeasurementCb::GnssData data;
    ASSERT_EQ(NotReady, obj.retrieveSvInfo(data));
}

TEST_F(GnssNavTimeUTCParserTest, checkRetrieveNotReadyInvalidWeekFlag)
{
    const uint8_t navTimeUtcInvalidFlag[] = {
        0x60, 0xee, 0x5e, 0x0c,
        0x0a, 0x00, 0x00, 0x00,
        0x31, 0xde, 0xfb, 0xff,
        0xe3, 0x07, 0x02, 0x1a,
        0x09, 0x26, 0x32, 0x35,
    };

    GnssNavTimeUTCParser obj(navTimeUtcInvalidFlag, (uint16_t)sizeof(navTimeUtcInvalidFlag));
    MeasurementCb::GnssData data;
    ASSERT_EQ(NotReady, obj.retrieveSvInfo(data));
}

TEST_F(GnssNavTimeUTCParserTest, checkRetrieveNotReadyInvalidUtcFlag)
{
    const uint8_t navTimeUtcInvalidFlag[] = {
        0x60, 0xee, 0x5e, 0x0c,
        0x0a, 0x00, 0x00, 0x00,
        0x31, 0xde, 0xfb, 0xff,
        0xe3, 0x07, 0x02, 0x1a,
        0x09, 0x26, 0x32, 0x33,
    };

    GnssNavTimeUTCParser obj(navTimeUtcInvalidFlag, (uint16_t)sizeof(navTimeUtcInvalidFlag));
    MeasurementCb::GnssData data;
    ASSERT_EQ(NotReady, obj.retrieveSvInfo(data));
}

TEST_F(GnssNavTimeUTCParserTest, DISABLED_checkRetrieveNotReadyInvalidITowFlag)
{

    const uint8_t navTimeUtcInvalidFlag[] = {
        0x60, 0xee, 0x5e, 0x0c,
        0x0a, 0x00, 0x00, 0x00,
        0x31, 0xde, 0xfb, 0xff,
        0xe3, 0x07, 0x02, 0x1a,
        0x09, 0x26, 0x32, 0x36,
    };

    GnssNavTimeUTCParser obj(navTimeUtcInvalidFlag, (uint16_t)sizeof(navTimeUtcInvalidFlag));
    MeasurementCb::GnssData data;
    ASSERT_EQ(UTCDone, obj.retrieveSvInfo(data));
}

TEST_F(GnssNavTimeUTCParserTest, checkRetrieveNotReadyInvalidZeroFlag)
{
    const uint8_t navTimeUtcInvalidFlag[] = {
        0x60, 0xee, 0x5e, 0x0c,
        0x0a, 0x00, 0x00, 0x00,
        0x31, 0xde, 0xfb, 0xff,
        0xe3, 0x07, 0x02, 0x1a,
        0x09, 0x26, 0x32, 0x00,
    };

    GnssNavTimeUTCParser obj(navTimeUtcInvalidFlag, (uint16_t)sizeof(navTimeUtcInvalidFlag));
    MeasurementCb::GnssData data;
    ASSERT_EQ(NotReady, obj.retrieveSvInfo(data));
}

