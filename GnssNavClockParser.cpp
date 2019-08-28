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

#define LOG_TAG "GnssHALNavClockParser"
#define LOG_NDEBUG 1

#include <log/log.h>
#include "GnssNavClockParser.h"


// Using offsets according to the protocol description of UBX-NAV-CLOCK
enum NavClockOffsets : uint8_t {
    iTow = 0,
    clockBias = 4,
    clockDrift = 8,
    timeAccuracy = 12,
    freqAccuracyEstimate = 16,
};

static const uint16_t blockSize = 20;

GnssNavClockParser::GnssNavClockParser(const uint8_t* payload, uint16_t payloadLen) :
    mPayload(payload),
    mPayloadLen(payloadLen)
{
    parseNavClockMsg();
    mPayload = nullptr;
    mPayloadLen = 0;
}

void GnssNavClockParser::parseSingleBlock()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    data.iTow = getValue<uint32_t>(&mPayload[NavClockOffsets::iTow]);
    data.clockBias = getValue<int32_t>(&mPayload[NavClockOffsets::clockBias]);
    data.clockDrift = getValue<int32_t>(&mPayload[NavClockOffsets::clockDrift]);
    data.timeAccuracy = getValue<uint32_t>(&mPayload[NavClockOffsets::timeAccuracy]);
    data.freqAccuracyEstimate = getValue<uint32_t>(&mPayload[NavClockOffsets::freqAccuracyEstimate]);

    mValid = true;
    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

void GnssNavClockParser::parseNavClockMsg()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (nullptr == mPayload || mPayloadLen != blockSize) {
        ALOGV("[%s, line %d] Payload is not valid", __func__, __LINE__);
        return;
    }

    parseSingleBlock();
    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

void GnssNavClockParser::getGnssClock(MeasurementCb::GnssClock &instance)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    const double psToNsScale = 1000.0;

    instance.biasNs = static_cast<int64_t>(data.clockBias);
    instance.driftNsps = static_cast<double>(data.clockDrift);
    instance.biasUncertaintyNs = static_cast<double>(data.timeAccuracy);
    instance.driftUncertaintyNsps = scaleDown(data.freqAccuracyEstimate, psToNsScale);
    instance.hwClockDiscontinuityCount = 0;

    instance.gnssClockFlags |= static_cast<uint16_t>(MeasurementCb::GnssClockFlags::HAS_BIAS);
    instance.gnssClockFlags |= static_cast<uint16_t>(MeasurementCb::GnssClockFlags::HAS_BIAS_UNCERTAINTY);
    instance.gnssClockFlags |= static_cast<uint16_t>(MeasurementCb::GnssClockFlags::HAS_DRIFT);
    instance.gnssClockFlags |= static_cast<uint16_t>(MeasurementCb::GnssClockFlags::HAS_DRIFT_UNCERTAINTY);
}

uint8_t GnssNavClockParser::retrieveSvInfo(MeasurementCb::GnssData &gnssData)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (mValid) {
        getGnssClock(gnssData.clock);
        ALOGV("[%s, line %d] Exit ClockDone", __func__, __LINE__);
        return ClockDone;
    }

    return NotReady;
}

void GnssNavClockParser::dumpDebug()
{
    hexdump("/data/app/NavClockSample", (void*)&data, sizeof(data));
    ALOGV("[%s, line %d] GetClock: timeNs %u, time accuracy: %u, bias: %d, drift: %d",
          __func__, __LINE__, data.iTow, data.timeAccuracy, data.clockBias, data.clockDrift);
}
