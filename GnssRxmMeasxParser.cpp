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
    gnssId = 0,
    svId = 1,
    cn0 = 2,
    multipath = 3,
    pseudorangeRate = 4,
    flags = 8,
    psRanRateCorr = 19,
};

GnssRxmMeasxParser::GnssRxmMeasxParser(const char *payload, uint16_t payloadLen) :
    mPayload((uint8_t*)payload),
    mPayloadLen(payloadLen)
{
    parseRxmMeasMsg();

    mPayload = nullptr;
    mPayloadLen = 0;;
}

void GnssRxmMeasxParser::parseSingleBlock(const uint8_t* msg)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    if (nullptr != msg) {
        meta.version = msg[RxmMeasxOffsets::version];
        meta.numSvs = msg[RxmMeasxOffsets::numSvs];

        meta.gpsTOW = getUint32(&msg[RxmMeasxOffsets::gpsTOW]);
        meta.glonassTOW = getUint32(&msg[RxmMeasxOffsets::glonassTOW]);
        meta.bdsTOW = getUint32(&msg[RxmMeasxOffsets::bdsTOW]);
        meta.qzssTOW = getUint32(&msg[RxmMeasxOffsets::qzssTOW]);

        meta.gpsTOWacc = getUint16(&msg[RxmMeasxOffsets::gpsTOWacc]);
        meta.glonassTOWacc = getUint16(&msg[RxmMeasxOffsets::glonassTOWacc]);
        meta.bdsTOWacc = getUint16(&msg[RxmMeasxOffsets::bdsTOWacc]);
        meta.qzssTOWacc = getUint16(&msg[RxmMeasxOffsets::qzssTOWacc]);

        meta.TOWset = msg[RxmMeasxOffsets::TOWset];
        ALOGV("[%s, line %d] Exit", __func__, __LINE__);
    }
}

void GnssRxmMeasxParser::parseRepeatedBlock(const uint8_t* msg)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    if (nullptr != msg) {
        repeatedBlock_t block;

        block.gnssId = msg[RxmMeasxOffsets::gnssId];
        block.svId = msg[RxmMeasxOffsets::svId];
        block.cn0 = msg[RxmMeasxOffsets::cn0];
        block.multipath = msg[RxmMeasxOffsets::multipath];
        block.pseudoRangeRate = getUint32(&msg[RxmMeasxOffsets::pseudorangeRate]);

        data.push_back(block);
        ALOGV("[%s, line %d] Exit", __func__, __LINE__);
    }
}

void GnssRxmMeasxParser::parseRepeatedBlocks(const uint8_t* msg,const uint16_t msgLen)
{
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

void GnssRxmMeasxParser::parseRxmMeasMsg()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (nullptr == mPayload || ((maxSvsNum * repeatedBlockSize) + singleBlockSize) < mPayloadLen) {
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

GnssConstellationType GnssRxmMeasxParser::getConstellationFromGnssId(const uint8_t gnssId)
{
    switch (gnssId) {
    case UbxGnssId::GPS:
        return GnssConstellationType::GPS;
    case UbxGnssId::GLONASS:
        return GnssConstellationType::GLONASS;
    case UbxGnssId::GALILEO:
        return GnssConstellationType::GALILEO;
    case UbxGnssId::QZSS:
        return GnssConstellationType::QZSS;
    case UbxGnssId::BEIDOU:
        return GnssConstellationType::BEIDOU;
    case UbxGnssId::SBAS:
        return GnssConstellationType::SBAS;
    default:
        return GnssConstellationType::UNKNOWN;
    }
}

uint32_t GnssRxmMeasxParser::getTOWforGnssId(const uint8_t gnssId)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    if (0 == meta.TOWset) {
        mTOWstate = GnssMS::STATE_UNKNOWN;
        ALOGV("[%s, line %d] TOW unknown", __func__, __LINE__);
        return 0;
    }

    mTOWstate = GnssMS::STATE_TOW_DECODED;

    switch (gnssId) {
    case UbxGnssId::GPS:
        return meta.gpsTOW;
    case UbxGnssId::GLONASS:
        return meta.glonassTOW;
    case UbxGnssId::QZSS:
        return meta.qzssTOW;
    case UbxGnssId::BEIDOU:
        return meta.bdsTOW;
    case UbxGnssId::SBAS:
    case UbxGnssId::GALILEO:
        mTOWstate = GnssMS::STATE_TOW_KNOWN;
        return meta.gpsTOW; //TODO: find better solution
    default:
        mTOWstate = GnssMS::STATE_UNKNOWN;
        return 0;
    }
}

uint16_t GnssRxmMeasxParser::getTOWaccForGnssId(const uint8_t gnssId)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    if (0 == meta.TOWset || GnssMS::STATE_UNKNOWN == mTOWstate) {
        ALOGV("[%s, line %d] TOW unknown", __func__, __LINE__);
        return 0;
    }

    switch (gnssId) {
    case UbxGnssId::GPS:
        return meta.gpsTOWacc;
    case UbxGnssId::GLONASS:
        return meta.glonassTOWacc;
    case UbxGnssId::QZSS:
        return meta.qzssTOWacc;
    case UbxGnssId::BEIDOU:
        return meta.bdsTOWacc;
    case UbxGnssId::SBAS:
    case UbxGnssId::GALILEO:
        return meta.gpsTOWacc; //TODO: find better solution
    default:
        return 0;
    }
}

uint8_t GnssRxmMeasxParser::getValidSvidForGnssId(const uint8_t gnssId, const uint8_t svid)
{
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
        inRanges(glonassFirst, glonassLast, glonassFcnFirst, glonassFcnLast, resultSvid);
        break;
    }

    ALOGV("[%s, line %d] svid = %u", __func__, __LINE__, resultSvid);
    return resultSvid;
}

void GnssRxmMeasxParser::getGnssMeasurement(IGnssMeasurementCallback::GnssMeasurement &instance, repeatedBlock_t &block)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    instance.flags = 0;

    instance.svid = getValidSvidForGnssId(block.gnssId, block.svId);
    instance.constellation = getConstellationFromGnssId(block.gnssId);

    instance.receivedSvTimeInNs = setNsFromMs(getTOWforGnssId(block.gnssId));
    instance.receivedSvTimeUncertaintyInNs = scaleDown(setNsFromMs(getTOWaccForGnssId(block.gnssId)), towAccScaleDown);
    instance.state = static_cast<uint32_t>(mTOWstate);

    instance.cN0DbHz = static_cast<double>(block.cn0);
    instance.multipathIndicator = (block.multipath == 0) ? GnssMI::INDICATIOR_NOT_PRESENT : GnssMI::INDICATOR_PRESENT;

    instance.pseudorangeRateMps = scaleUp(block.pseudoRangeRate, pseudorangeRateScaleUp);
    instance.pseudorangeRateUncertaintyMps = 1.0; // TODO: set real value, at moment it is pure magic.

    instance.accumulatedDeltaRangeState = static_cast<uint16_t>(GnssADRS::ADR_STATE_UNKNOWN);

    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

uint8_t GnssRxmMeasxParser::retrieveSvInfo(android::hardware::gnss::V1_0::IGnssMeasurementCallback::GnssData &gnssData)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    if (mValid) {
        uint32_t measurementCount = 0;
        for (auto &block : data) {
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

int64_t GnssRxmMeasxParser::setNsFromMs(uint32_t ms)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    const uint32_t msToNsMultiplier = 1000000;
    return static_cast<int64_t>(ms * msToNsMultiplier);

}

void GnssRxmMeasxParser::dumpDebug()
{
    ALOGV("[%s, line %d] ", __func__, __LINE__);
    const char* file = "/data/app/RxmMeasxParser_dump";
    hexdump(file, (void*)&meta, sizeof(meta));

    for (auto block : data) {
        hexdump(file, (void*)&block, sizeof(repeatedBlock_t));
        ALOGV("[%s, line %d] svid: %u, constel: %u, state: %u, cN0DbHz: %u, mpath: %u, pRangeRate: %f",
              __func__, __LINE__, block.svId, (uint16_t)getConstellationFromGnssId(block.gnssId),
              (uint32_t)mTOWstate, block.cn0, (uint16_t)block.multipath,
              scaleUp(block.pseudoRangeRate, pseudorangeRateScaleUp));
    }
}
