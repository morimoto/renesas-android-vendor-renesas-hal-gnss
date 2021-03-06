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

#define LOG_TAG "GnssHALRxmMeasxParser"
#define LOG_NDEBUG 1

#include <log/log.h>
#include "GnssRxmMeasxParser.h"

static const uint8_t maxSvsNum = 64;
static const uint16_t repeatedBlockSize = 24;
static const uint16_t singleBlockSize = 44;
static const int64_t towAccScaleDown = 16;
static const double pseudorangeRateScaleUp = 0.04;
static const int64_t msToNsMultiplier = 1000000;

// Satellite vehicle numbering according to documentation
static const uint8_t gpsFirst = 1;
static const uint8_t gpsLast = 32;
static const uint8_t sbasOneFirst = 120;
static const uint8_t sbasOneLast = 151;
static const uint8_t sbasTwoFirst = 183;
static const uint8_t sbasTwoLast = 192;
static const uint8_t galileoFirst = 1;
static const uint8_t galileoLast = 36;
static const uint8_t qzssFirst = 193;
static const uint8_t qzssLast = 200;
static const uint8_t bdFirst = 1;
static const uint8_t bdLast = 37;
static const uint8_t glonassFcnFirst = 93;
static const uint8_t glonassFcnLast = 106;
static const uint8_t glonassFirst = 1;
static const uint8_t glonassLast = 24;

// Using offsets according to the protocol description of UBX-RXM-MEASX
enum RxmMeasxOffsets : uint8_t {
    //first block offsets
    version = 0,
    gpsTOW = 4,
    glonassTOW = 8,
    bdsTOW = 12,
    qzssTOW = 20,
    gpsTOWacc = 24,
    glonassTOWacc = 26,
    bdsTOWacc = 28,
    qzssTOWacc = 32,
    numSvs = 34,
    TOWset = 35,

    //repeated block offsets
    gnssIdOfst = 0,
    svId = 1,
    cn0 = 2,
    multipath = 3,
    pseudorangeRate = 4,
};

GnssRxmMeasxParser::GnssRxmMeasxParser(const uint8_t* payload,
                                       uint16_t payloadLen) :
    mPayload(payload),
    mPayloadLen(payloadLen) {
    parseRxmMeasMsg();
    mPayload = nullptr;
    mPayloadLen = 0;;
}

void GnssRxmMeasxParser::parseSingleBlock(const uint8_t* msg) {
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (nullptr != msg) {
        meta.version = msg[RxmMeasxOffsets::version];
        meta.numSvs = msg[RxmMeasxOffsets::numSvs];
        meta.gpsTOW = getValue<uint32_t>(&msg[RxmMeasxOffsets::gpsTOW]);
        meta.glonassTOW = getValue<uint32_t>(&msg[RxmMeasxOffsets::glonassTOW]);
        meta.bdsTOW = getValue<uint32_t>(&msg[RxmMeasxOffsets::bdsTOW]);
        meta.qzssTOW = getValue<uint32_t>(&msg[RxmMeasxOffsets::qzssTOW]);
        meta.gpsTOWacc = getValue<uint16_t>(&msg[RxmMeasxOffsets::gpsTOWacc]);
        meta.glonassTOWacc = getValue<uint16_t>(&msg[RxmMeasxOffsets::glonassTOWacc]);
        meta.bdsTOWacc = getValue<uint16_t>(&msg[RxmMeasxOffsets::bdsTOWacc]);
        meta.qzssTOWacc = getValue<uint16_t>(&msg[RxmMeasxOffsets::qzssTOWacc]);
        meta.TOWset = msg[RxmMeasxOffsets::TOWset];
        ALOGV("[%s, line %d] Exit", __func__, __LINE__);
    }
}

void GnssRxmMeasxParser::parseRepeatedBlock(const uint8_t* msg) {
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (nullptr != msg) {
        repeatedBlock_t block;
        block.gnssId = msg[RxmMeasxOffsets::gnssIdOfst];
        block.svId = msg[RxmMeasxOffsets::svId];
        block.cn0 = msg[RxmMeasxOffsets::cn0];
        block.multipath = msg[RxmMeasxOffsets::multipath];
        block.pseudoRangeRate = getValue<int32_t>
                                (&msg[RxmMeasxOffsets::pseudorangeRate]);
        data.push_back(block);
        ALOGV("[%s, line %d] Exit", __func__, __LINE__);
    }
}

void GnssRxmMeasxParser::parseRepeatedBlocks(const uint8_t* msg,
        const uint16_t msgLen) {
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (nullptr == msg) {
        ALOGV("[%s, line %d] nullptr input parameter", __func__, __LINE__);
        return;
    }

    uint16_t offset = 0;

    for (uint8_t i = 0; i < meta.numSvs; i++) {
        if (offset + repeatedBlockSize <= msgLen) {
            parseRepeatedBlock(msg + offset);
            offset += repeatedBlockSize;
            mValid = true;
        }
    }

    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

void GnssRxmMeasxParser::parseRxmMeasMsg() {
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (nullptr == mPayload
        || ((maxSvsNum * repeatedBlockSize) + singleBlockSize) < mPayloadLen) {
        ALOGV("[%s, line %d] mPayload is null", __func__, __LINE__);
        return;
    }

    if (mPayloadLen >= singleBlockSize + repeatedBlockSize) {
        parseSingleBlock(mPayload);

        if (meta.numSvs <= maxSvsNum) {
            parseRepeatedBlocks(mPayload + singleBlockSize, mPayloadLen - singleBlockSize);
        }

        ALOGV("[%s, line %d] Exit", __func__, __LINE__);
    }
}

CnstlType GnssRxmMeasxParser::getConstellationFromGnssId(const uint8_t gnssId) {
    switch (gnssId) {
    case UbxGnssId::GPS:
        return CnstlType::GPS;

    case UbxGnssId::GLONASS:
        return CnstlType::GLONASS;

    case UbxGnssId::GALILEO:
        return CnstlType::GALILEO;

    case UbxGnssId::QZSS:
        return CnstlType::QZSS;

    case UbxGnssId::BEIDOU:
        return CnstlType::BEIDOU;

    case UbxGnssId::SBAS:
        return CnstlType::SBAS;

    default:
        return CnstlType::UNKNOWN;
    }
}

uint32_t GnssRxmMeasxParser::getTOWforGnssId(const uint8_t gnssId) {
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (0 == meta.TOWset) {
        mTOWstate = MeasurementState::STATE_UNKNOWN;
        ALOGV("[%s, line %d] TOW unknown", __func__, __LINE__);
        return 0;
    }

    uint32_t result = 0;
    mTOWstate = MeasurementState::STATE_TOW_DECODED;

    switch (gnssId) {
    case UbxGnssId::GPS:
        result = meta.gpsTOW;
        break;

    case UbxGnssId::GLONASS:
        result = meta.glonassTOW;
        break;

    case UbxGnssId::QZSS:
        result = meta.qzssTOW;
        break;

    case UbxGnssId::BEIDOU:
        result = meta.bdsTOW;
        break;

    case UbxGnssId::SBAS:
    case UbxGnssId::GALILEO:
        mTOWstate = MeasurementState::STATE_TOW_KNOWN;
        result = meta.gpsTOW; //TODO: find better solution
        break;
    }

    if (0 == result) {
        mTOWstate = MeasurementState::STATE_UNKNOWN;
    } else {
        mTOWstate = static_cast<MeasurementState>(mTOWstate |
                    MeasurementState::STATE_BIT_SYNC | MeasurementState::STATE_SUBFRAME_SYNC);
    }

    return result;
}

int64_t GnssRxmMeasxParser::getTOWaccForGnssId(const uint8_t gnssId) {
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (0 == meta.TOWset || MeasurementState::STATE_UNKNOWN == mTOWstate) {
        ALOGV("[%s, line %d] TOW unknown", __func__, __LINE__);
        return 0;
    }

    uint16_t towAccMs  = 0;

    switch (gnssId) {
    case UbxGnssId::GPS:
        towAccMs = meta.gpsTOWacc;
        break;

    case UbxGnssId::GLONASS:
        towAccMs = meta.glonassTOWacc;
        break;

    case UbxGnssId::QZSS:
        towAccMs = meta.qzssTOWacc;
        break;

    case UbxGnssId::BEIDOU:
        towAccMs = meta.bdsTOWacc;
        break;

    case UbxGnssId::SBAS:
    case UbxGnssId::GALILEO:
        towAccMs = meta.gpsTOWacc; //TODO: find better solution
        break;

    default:
        mTOWstate = MeasurementState::STATE_UNKNOWN;
        return 0;
    }

    int64_t result = scaleUp(scaleDown(towAccMs, towAccScaleDown),
                             msToNsMultiplier);
    return ((result > 0) ? result : 1);
}

uint8_t GnssRxmMeasxParser::getValidSvidForGnssId(const uint8_t gnssId,
        const uint8_t svid) {
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    uint8_t resultSvid = svid;

    switch (gnssId) {
    case UbxGnssId::GPS:
        inRange(gpsFirst, gpsLast, resultSvid);
        break;

    case UbxGnssId::SBAS:
        inRanges(sbasOneFirst, sbasOneLast, sbasTwoFirst, sbasTwoLast, resultSvid);
        break;

    case UbxGnssId::GALILEO:
        inRange(galileoFirst, galileoLast, resultSvid);
        break;

    case UbxGnssId::QZSS:
        inRange(qzssFirst, qzssLast, resultSvid);
        break;

    case UbxGnssId::BEIDOU:
        inRange(bdFirst, bdLast, resultSvid);
        break;

    case UbxGnssId::GLONASS:
        inRanges(glonassFirst, glonassLast, glonassFcnFirst, glonassFcnLast,
                 resultSvid);
        break;
    }

    ALOGV("[%s, line %d] svid = %u", __func__, __LINE__, resultSvid);
    return resultSvid;
}

float GnssRxmMeasxParser::getCarrierFrequencyFromGnssId(const uint8_t gnssId) {
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    const float L1BandFrequency = 1575.42f;
    const float B1BandFrequency = 1561.098f;
    const float L1GlonassBandFrequency = 1602.562f;
    const float scale = 1000000.0;

    switch (gnssId) {
    case UbxGnssId::GPS:
    case UbxGnssId::SBAS:
    case UbxGnssId::GALILEO:
    case UbxGnssId::QZSS:
        return scaleUp(L1BandFrequency, scale);

    case UbxGnssId::BEIDOU:
        return scaleUp(B1BandFrequency, scale);

    case UbxGnssId::GLONASS:
        return scaleUp(L1GlonassBandFrequency, scale);

    default:
        return 0;
    }
}

void GnssRxmMeasxParser::getGnssMeasurement(MeasurementCb::GnssMeasurement&
        instance,
        const repeatedBlock_t& block) {
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    instance.carrierFrequencyHz = getCarrierFrequencyFromGnssId(block.gnssId);
    instance.flags = 0;

    if (instance.carrierFrequencyHz > 0.0f) {
        instance.flags = static_cast<uint32_t>(MeasurementFlags::HAS_CARRIER_FREQUENCY);
    }

    instance.svid = getValidSvidForGnssId(block.gnssId, block.svId);
    instance.constellation = getConstellationFromGnssId(block.gnssId);
    instance.receivedSvTimeInNs = scaleUp(getTOWforGnssId(block.gnssId),
                                          msToNsMultiplier);
    instance.receivedSvTimeUncertaintyInNs = getTOWaccForGnssId(block.gnssId);
    instance.state = static_cast<uint32_t>(mTOWstate);
    instance.cN0DbHz = static_cast<double>(block.cn0);
    instance.multipathIndicator = (block.multipath == 0) ?
                                  MultipathId::INDICATIOR_NOT_PRESENT : MultipathId::INDICATOR_PRESENT;
    instance.pseudorangeRateMps = scaleUp(block.pseudoRangeRate,
                                          pseudorangeRateScaleUp);
    instance.pseudorangeRateUncertaintyMps =
        0.075; // TODO: set real value, at moment it is pure magic.
    instance.accumulatedDeltaRangeState = static_cast<uint16_t>
                                          (AccumulatedDeltaRangeState::ADR_STATE_UNKNOWN);
    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

uint8_t GnssRxmMeasxParser::retrieveSvInfo(MeasurementCb::GnssData& gnssData) {
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (mValid) {
        uint32_t measurementCount = 0;

        for (auto& block : data) {
            if (measurementCount < maxSvsNum) {
                getGnssMeasurement(gnssData.measurements[measurementCount], block);
                ++measurementCount;
            }
        }

        gnssData.measurementCount = measurementCount;
        ALOGV("[%s, line %d] Exit Done", __func__, __LINE__);
        return RxmDone;
    }

    return NotReady;
}

void GnssRxmMeasxParser::dumpDebug() {
    ALOGV("[%s, line %d] ", __func__, __LINE__);
    const char* file = "/data/app/RxmMeasxParser_dump";
    hexdump(file, (void*)&meta, sizeof(meta));

    for (auto block : data) {
        hexdump(file, (void*)&block, sizeof(repeatedBlock_t));
        ALOGV("[%s, line %d] svid: %u, constel: %u, state: %u, cN0DbHz: %u, mpath: %u, pRangeRate: %f",
              __func__, __LINE__, block.svId,
              (uint16_t)getConstellationFromGnssId(block.gnssId),
              (uint32_t)mTOWstate, block.cn0, (uint16_t)block.multipath,
              scaleUp(block.pseudoRangeRate, pseudorangeRateScaleUp));
    }
}
