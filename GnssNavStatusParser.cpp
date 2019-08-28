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

#define LOG_TAG "GnssHALNavStatusParser"
#define LOG_NDEBUG 1

#include <log/log.h>
#include "GnssNavStatusParser.h"

static const size_t blockSize = 16;
static const int64_t msToNs = 1000000;
static const uint16_t fullBiasFlag = static_cast<uint16_t>(MeasurementCb::GnssClockFlags::HAS_FULL_BIAS);

// Using offsets according to the protocol description of UBX-NAV-STATUS
enum NavStatusOffsets : uint8_t {
    iTowOffset = 0,
    msssOffset = 12,
};

GnssNavStatusParser::GnssNavStatusParser(const uint8_t* payload, uint16_t payloadLen) :
    mPayload(payload),
    mPayloadLen(payloadLen)
{
    parseNavStatusMsg();
    mPayload = nullptr;
    mPayloadLen = 0;
}

void GnssNavStatusParser::parseSingleBlock()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    data.iTow = getValue<uint32_t>(&mPayload[NavStatusOffsets::iTowOffset]);
    data.msss = getValue<uint32_t>(&mPayload[NavStatusOffsets::msssOffset]);

    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

void GnssNavStatusParser::parseNavStatusMsg()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (nullptr == mPayload || mPayloadLen != blockSize) {
        ALOGV("[%s, line %d] Payload is not valid", __func__, __LINE__);
        return;
    }

    parseSingleBlock();
    mValid = setTimeNano();

    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

bool GnssNavStatusParser::setTimeNano()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    timeNano = scaleUp(data.msss, msToNs);

    ALOGV("[%s, line %d] timeNano %ld", __func__, __LINE__, timeNano);
    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
    return true;
}

static bool hasFlagFullBias(const uint16_t flags)
{
    return (fullBiasFlag == (fullBiasFlag & flags));
}

uint8_t GnssNavStatusParser::retrieveSvInfo(MeasurementCb::GnssData &gnssData)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (mValid) {
        if (hasFlagFullBias(gnssData.clock.gnssClockFlags)) {
            ALOGV("[%s, line %d] has full bias", __func__, __LINE__);
            gnssData.clock.fullBiasNs -= (gnssData.clock.timeNs - timeNano);
        }

        gnssData.clock.timeNs = timeNano;

        ALOGV("[%s, line %d] Exit", __func__, __LINE__);
        return StatusDone;
    }

    return NotReady;
}

void GnssNavStatusParser::dumpDebug()
{
    ALOGD("Time since startup = %ld", timeNano);
}

