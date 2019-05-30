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

#define LOG_TAG "GnssHALNavTimeGPSParser"
#define LOG_NDEBUG 1

#include <log/log.h>
#include "GnssNavTimeGPSParser.h"

static const size_t blockSize = 16;
static const int64_t fullWeekMs = 604800000; // ms in week
static const int64_t defaultRtcTime = 1;

static bool isValidFlag(const uint8_t flags, const uint8_t expFlag);

// Using offsets according to the protocol description of UBX-NAV-TIMEGPS
enum NavTimeGpsOffsets : uint8_t {
    iTowOffset = 0,
    fTowOffset = 4,
    weekOffset = 8,
    leapSOffset = 10,
    validOffset = 11,
    tAccOffset = 12,
};

GnssNavTimeGPSParser::GnssNavTimeGPSParser(const char *payload, uint16_t payloadLen) :
    mPayload((uint8_t*)payload),
    mPayloadLen(payloadLen)
{
    parseNavTimeGPSMsg();
    mPayload = nullptr;
    mPayloadLen = 0;
}

void GnssNavTimeGPSParser::parseSingleBlock()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    data.iTow = getUint32(&mPayload[NavTimeGpsOffsets::iTowOffset]);
    data.fTow = getInt32(&mPayload[NavTimeGpsOffsets::fTowOffset]);
    data.week = getInt16(&mPayload[NavTimeGpsOffsets::weekOffset]);
    data.leapS = static_cast<int8_t>(mPayload[NavTimeGpsOffsets::leapSOffset]);
    data.valid = mPayload[NavTimeGpsOffsets::validOffset];
    data.tAcc = getUint32(&mPayload[NavTimeGpsOffsets::tAccOffset]);

    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

void GnssNavTimeGPSParser::parseNavTimeGPSMsg()
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

bool GnssNavTimeGPSParser::setTimeNano()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    const int64_t msToNs = 1000000;
    int64_t tmpGpsTimeNs = scaleUp(data.week * fullWeekMs, msToNs);
    int64_t tmpTowNs = scaleUp(data.iTow, msToNs) + data.fTow;

    timeNano = tmpGpsTimeNs + tmpTowNs;

    ALOGV("[%s, line %d] timeNano %ld", __func__, __LINE__, timeNano);
    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
    return true;
}

uint8_t GnssNavTimeGPSParser::retrieveSvInfo(IGnssMeasurementCallback::GnssData &gnssData)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    if (mValid) {
        int64_t updateTimeNs = std::max(gnssData.clock.timeNs, defaultRtcTime);

        gnssData.clock.timeNs = updateTimeNs;
        gnssData.clock.fullBiasNs = updateTimeNs - timeNano;
        gnssData.clock.timeUncertaintyNs = static_cast<double>(data.tAcc);

        gnssData.clock.gnssClockFlags =
                static_cast<uint16_t>(IGnssMeasurementCallback::GnssClockFlags::HAS_TIME_UNCERTAINTY);
        gnssData.clock.gnssClockFlags |= static_cast<uint16_t>(IGnssMeasurementCallback::GnssClockFlags::HAS_FULL_BIAS);

        const uint8_t validLeapMask = 0x04;
        if (isValidFlag(data.valid, validLeapMask)) {
            gnssData.clock.leapSecond = data.leapS;
            gnssData.clock.gnssClockFlags |=
                    static_cast<uint16_t>(IGnssMeasurementCallback::GnssClockFlags::HAS_LEAP_SECOND);
        }

        ALOGV("[%s, line %d] Exit", __func__, __LINE__);
        return GPSTimeDone;
    }

    return NotReady;
}

void GnssNavTimeGPSParser::dumpDebug()
{
    ALOGD("Time nano = %ld, leap second = %d", timeNano, data.leapS);
}

static bool isValidFlag(const uint8_t flags, const uint8_t expFlag)
{
    uint8_t val = (flags & expFlag);
    if (expFlag == val) {
        return true;
    }
    return false;
}

bool GnssNavTimeGPSParser::checkFlags()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    const uint8_t validTowMask = 0x01;
    const uint8_t validWeekMask = 0x02;

    bool validTow = isValidFlag(data.valid, validTowMask);
    bool validWeek = isValidFlag(data.valid, validWeekMask);

    return (validWeek && validTow);
}
