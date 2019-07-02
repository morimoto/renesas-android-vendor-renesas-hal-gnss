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

#define LOG_TAG "GnssHALNavTimeUTCParser"
#define LOG_NDEBUG 1

#include <log/log.h>
#include <ctime>
#include "GnssNavTimeUTCParser.h"

static const uint16_t blockSize = 20;
static const int xxCentury = 1900;
static const int january = 1;
static const int millenium = 100;  //xxCentury + millenium corresponds to 2000 year in tm.tm_year value range
static const int64_t secondToNanoMultiplier = 1000 * 1000 * 1000;

// Using offsets according to the protocol description of UBX-NAV-TIMEUTC
enum NavTimeUTCOffsets : uint8_t {
    iTow = 0,
    timeAccuracy = 4,
    nanoSecondFraction = 8,
    year = 12,
    month = 14,
    day = 15,
    hour = 16,
    minute = 17,
    second = 18,
    flags = 19,
};

GnssNavTimeUTCParser::GnssNavTimeUTCParser(const uint8_t* payload, uint16_t payloadLen) :
    mPayload(payload),
    mPayloadLen(payloadLen)
{
    parseNavTimeUTCMsg();
    mPayload = nullptr;
    mPayloadLen = 0;
}

void GnssNavTimeUTCParser::parseSingleBlock()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    data.iTow = getValue<uint32_t>(&mPayload[NavTimeUTCOffsets::iTow]);
    data.timeAccuracy = getValue<uint32_t>(&mPayload[NavTimeUTCOffsets::timeAccuracy]);
    data.nanoSecondFraction = getValue<int32_t>(&mPayload[NavTimeUTCOffsets::nanoSecondFraction]);
    data.year = getValue<uint16_t>(&mPayload[NavTimeUTCOffsets::year]);
    data.month = mPayload[NavTimeUTCOffsets::month];
    data.day = mPayload[NavTimeUTCOffsets::day];
    data.hour = mPayload[NavTimeUTCOffsets::hour];
    data.minute = mPayload[NavTimeUTCOffsets::minute];
    data.second = mPayload[NavTimeUTCOffsets::second];
    data.flags = mPayload[NavTimeUTCOffsets::flags];

    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

void GnssNavTimeUTCParser::parseNavTimeUTCMsg()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (nullptr == mPayload || mPayloadLen != blockSize) {
        ALOGV("[%s, line %d] Payload is not valid", __func__, __LINE__);
        return;
    }

    parseSingleBlock();

    if (checkFlags()) {
       mValid = setTimeNano();
    }

    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

bool GnssNavTimeUTCParser::setTimeNano()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    tm recTime {
        .tm_sec = 0,
        .tm_min = 0,
        .tm_hour = 0,
        .tm_mday = 0,
        .tm_mon = 0,
        .tm_year = 0,
        .tm_wday = 0,
        .tm_yday = 0,
        .tm_isdst = -1,
        .tm_gmtoff = 0,
        .tm_zone = nullptr,
    };

    int curYear = static_cast<int>(data.year) - xxCentury;
    if (curYear < millenium) {
        ALOGV("[%s, line %d] Invalid year %d", __func__, __LINE__, curYear);
        return false;
    }

    int curMonth = static_cast<int>(data.month) - january;
    // 0 - for January, 11 - for December
    if (curMonth < 0 || curMonth > 11) {
        ALOGV("[%s, line %d] Invalid month %d", __func__, __LINE__, curMonth);
        return false;
    }

    recTime.tm_year = curYear;
    recTime.tm_mon = curMonth;
    recTime.tm_mday = static_cast<int>(data.day);
    recTime.tm_hour = static_cast<int>(data.hour);
    recTime.tm_min = static_cast<int>(data.minute);
    recTime.tm_sec = static_cast<int>(data.second);

    time_t time = std::mktime(&recTime);
    if (time < 0) {
        ALOGV("Invalid time");
        return false;
    }

    timeNano = static_cast<int64_t>(time * secondToNanoMultiplier);
    timeNano += data.nanoSecondFraction;

    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
    return true;
}

uint8_t GnssNavTimeUTCParser::retrieveSvInfo(IGnssMeasurementCallback::GnssData &gnssData __unused)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    return NotReady;
}

void GnssNavTimeUTCParser::dumpDebug()
{
    hexdump("/data/app/NavTimeUTCParser",(void*)&data, sizeof(data));
    ALOGV("[%s, line %d] iTOW %u, year %u, month %u, day %u, hour %u, minute %u, second %u, nanosec %u, timeAcc %u,"
          " flags %u", __func__, __LINE__, data.iTow, data.year, data.month, data.day, data.hour, data.minute,
          data.second, data.nanoSecondFraction, data.timeAccuracy, data.flags);
}

static bool isValidFlag(const uint8_t flags, const uint8_t expFlag)
{
    uint8_t val = (flags & expFlag);
    if (expFlag == val) {
        return true;
    }
    return false;
}

static uint8_t getUtcStandard(const uint8_t flag)
{
    const uint8_t defRet = 15; // default for Unknown standard
    const uint8_t rifghtShift = 4;

    uint8_t res = (flag >> rifghtShift);
    if (res < defRet) {
       return res;
    }

    return defRet;
}

bool GnssNavTimeUTCParser::checkFlags()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    const uint8_t validITowMask = 0x01;
    const uint8_t validWeekMask = 0x02;
    const uint8_t validUtcMask = 0x04;

    uint8_t UTCStandardId = getUtcStandard(data.flags);
    bool validITow = isValidFlag(data.flags, validITowMask);
    bool validWeek = isValidFlag(data.flags, validWeekMask);
    bool validUtc = isValidFlag(data.flags, validUtcMask);
    (void)validITow;

    ALOGV("[%s, line %d] flags: tow %u, week %u, utc %u, standard %u",
          __func__, __LINE__, validITow, validWeek, validUtc, UTCStandardId);

    return (validWeek && validUtc);
}
