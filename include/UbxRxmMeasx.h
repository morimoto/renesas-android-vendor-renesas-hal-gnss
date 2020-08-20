/*
 * Copyright (C) 2020 GlobalLogic
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

#ifndef UBXRXMMEASX_H
#define UBXRXMMEASX_H

#include <android/hardware/gnss/2.0/types.h>
#include <log/log.h>

#include <vector>

#include "include/UbxParserCommon.h"

/* From u-blox 8 / u-blox M8 Receiver Description - Manual
 * 33.18.2 UBX-RXM-MEASX (0x02 0x14)
 * 33.18.2.1 Satellite Measurements for RRLP
 * Message: UBX-RXM-MEASX
 * Description: Satellite Measurements for RRLP
*/

using ::android::hardware::gnss::V2_0::IGnssMeasurementCallback;
using ::android::hardware::gnss::V2_0::GnssConstellationType;

typedef IGnssMeasurementCallback::GnssMeasurementState GnssMs;
typedef IGnssMeasurementCallback::GnssMultipathIndicator GnssMI;
typedef IGnssMeasurementCallback::GnssAccumulatedDeltaRangeState GnssADRS;
typedef IGnssMeasurementCallback::GnssMeasurementFlags GnssMF;
typedef IGnssMeasurementCallback::GnssMeasurement GnssMeasx;

template <typename ClassType>
class UbxRxmMeasx : public UbxParserCommon<ClassType> {
public:
    UbxRxmMeasx(const char* in, const size_t& inLen);
    UbxRxmMeasx();
    ~UbxRxmMeasx() override;

    UbxMsg GetMsgType() override;
    UPError GetData(ClassType out) override;
    bool IsValid() override;

protected:
    UPError Parse();
    UPError ParseSingleBlock(const uint8_t* in);
    UPError ParseRepeatedBlocks(const uint8_t* in, const size_t inLen);
    UPError ParseRepeatedBlock(const uint8_t* in);
    UPError ValidateParcel();

    /*!
     * \brief GetTOWforGnssId - provide corresponding value (TOW)
     *        to the concrete constellation from meta private member
     * \brief set mTOWstate GnssMs::STATE_TOW_KNOWN
     *        if requested constellation is GALILEO or SBAS
     * \brief set mTOWstate GnssMs::STATE_UNKNOWN
     *        if requested constellation unknown
     * \param inGnssId - constellation id of UbxGnssId enum
     * \param outTow - time of week (TOW) in ms for concrete constellation,
     *                 otherwise 0  for unknown gnssId
     * \return Success, otherwise error code
     */
    UPError GetTowForGnssId(const UbxGnssId inGnssId, int64_t& outTow,
                            uint32_t& outState);

    /*!
     * \brief GetTOWforGnssId - provide corresponding value (TOWacc)
     *        to the concrete constellation from meta private member
     * \param inGnssId - constellation id of UbxGnssId enum
     * \param outTowAcc - TOW accuracy in ns
     * \param state - tow state
     * \return Success, otherwise error code
     */
    UPError GetTowAccForGnssId(const UbxGnssId inGnssId, int64_t& outTowAcc,
                               uint32_t& state);

    /*!
     * \brief GetConstellationFromGnssId - convert UbxGnssId
     *        to GnssConstellationType
     * \param inGnssId - index of constellation in means of UbxGnssId
     * \return constellation in means of GnssConstellationType
     */
    GnssConstellationType GetConstellationFromGnssId(const UbxGnssId inGnssId);

    /*!
     * \brief GetValidSvidForGnssId - validate svid
     * \param inGnssId - correspoing index of constelleation
     *                   in means of UbxGnssId
     * \param svid - satellite vehicle id provided by receiver
     * \param svid - set valid svid, in case of incorrectness
     *               provide any known svid for current constellation
     *         or frequency channel number for GLONASS
     * \return Success, otherwise error code
     */
    uint8_t GetValidSvidForGnssId(const UbxGnssId inGnssId, uint8_t svid);

    float GetCarrierFrequencyFromGnssId(const UbxGnssId inGnssId);

private:
    typedef struct SingleBlock {
        uint8_t version;
        uint8_t numSvs;
        uint32_t gpsTOW;
        uint32_t glonassTOW;
        uint32_t bdsTOW;
        uint32_t qzssTOW;
        uint16_t gpsTOWacc;
        uint16_t glonassTOWacc;
        uint16_t bdsTOWacc;
        uint16_t qzssTOWacc;
        uint8_t TOWset;
    } singleBlock_t;

    typedef struct RepeatedBlock {
        uint8_t gnssId;
        uint8_t svId;
        uint8_t cn0;
        uint8_t multipath;
        int32_t pseudoRangeRate;
    } repeatedBlock_t;

    typedef struct Parcel {
        singleBlock_t single;
        std::vector<repeatedBlock_t> repeated;
    } parcel_t;

    // Satellite vehicle numbering according to documentation
    enum SatelliteNumbers {
        gpsFirst = 1,
        gpsLast = 32,
        sbasOneFirst = 120,
        sbasOneLast = 151,
        sbasTwoFirst = 183,
        sbasTwoLast = 192,
        galileoFirst = 1,
        galileoLast = 36,
        qzssFirst = 193,
        qzssLast = 200,
        bdFirst = 1,
        bdLast = 37,
        glonassFcnFirst = 93,
        glonassFcnLast = 106,
        glonassFirst = 1,
        glonassLast = 24,
    };

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
    };

    enum ParcelSizeInfo {
        maxSvsNum = 64,
        repeatedBlockSize = 24,
        singleBlockSize = 44,
    };

    static constexpr double scalePrrFactor = 0.04;
    static constexpr int64_t scaleTowAcc = 16;
    static constexpr UbxMsg mType = UbxMsg::RXM_MEASX;

    UPError GetGnssMeasurement(repeatedBlock_t& block, GnssMeasx& measurement);

    const uint8_t* mPayload;
    const size_t mPayloadLen;

    bool mIsValid = false;
    parcel_t mParcel = {};
};

template <typename ClassType>
UbxRxmMeasx<ClassType>::UbxRxmMeasx() :
    mPayload(nullptr),
    mPayloadLen(0) {
}

template <typename ClassType>
UbxRxmMeasx<ClassType>::UbxRxmMeasx(const char* in, const size_t& inLen) :
    mPayload(reinterpret_cast<const uint8_t*>(in)),
    mPayloadLen(inLen) {
    if (UPError::Success == Parse()) {
        mIsValid = true;
    }
}

template <typename ClassType>
UPError UbxRxmMeasx<ClassType>::Parse() {
    UPError result = UPError::IncompletePacket;

    if (nullptr == mPayload ||
        ((maxSvsNum * repeatedBlockSize) + singleBlockSize) < mPayloadLen) {
        return result;
    }

    if (mPayloadLen >= singleBlockSize + repeatedBlockSize) {
        result = ParseSingleBlock(mPayload);

        if (UPError::Success != result || mParcel.single.numSvs > maxSvsNum) {
            return result;
        }

        result = ParseRepeatedBlocks(mPayload + singleBlockSize,
                                     mPayloadLen - singleBlockSize);

        if (UPError::Success == result) {
            result = ValidateParcel();
        }
    }

    return result;
}

template <typename ClassType>
UPError UbxRxmMeasx<ClassType>::ParseSingleBlock(const uint8_t* in) {
    UPError result = UPError::BadInputParameter;

    if (nullptr != in) {
        mParcel.single.version = in[RxmMeasxOffsets::version];
        mParcel.single.numSvs = in[RxmMeasxOffsets::numSvs];
        mParcel.single.gpsTOW = this->template GetValue<uint32_t>
        (&in[RxmMeasxOffsets::gpsTOW]);
        mParcel.single.glonassTOW = this->template GetValue<uint32_t>
        (&in[RxmMeasxOffsets::glonassTOW]);
        mParcel.single.bdsTOW = this->template GetValue<uint32_t>
        (&in[RxmMeasxOffsets::bdsTOW]);
        mParcel.single.qzssTOW = this->template GetValue<uint32_t>
        (&in[RxmMeasxOffsets::qzssTOW]);
        mParcel.single.gpsTOWacc = this->template GetValue<uint16_t>
        (&in[RxmMeasxOffsets::gpsTOWacc]);
        mParcel.single.glonassTOWacc = this->template GetValue<uint16_t>
        (&in[RxmMeasxOffsets::glonassTOWacc]);
        mParcel.single.bdsTOWacc = this->template GetValue<uint16_t>
        (&in[RxmMeasxOffsets::bdsTOWacc]);
        mParcel.single.qzssTOWacc = this->template GetValue<uint16_t>
        (&in[RxmMeasxOffsets::qzssTOWacc]);
        mParcel.single.TOWset = in[RxmMeasxOffsets::TOWset];
        result = UPError::Success;
    }

    return result;
}

template <typename ClassType>
UPError UbxRxmMeasx<ClassType>::ParseRepeatedBlocks(const uint8_t* in,
        const size_t inLen) {
    UPError result = UPError::BadInputParameter;

    if (nullptr == in) {
        return result;
    }

    size_t offset = 0;

    for (uint8_t i = 0; i < mParcel.single.numSvs; ++i) {
        if (offset + repeatedBlockSize <= inLen) {
            result = ParseRepeatedBlock(in + offset);

            if (UPError::Success != result) {
                break;
            }

            offset += repeatedBlockSize;
        }
    }

    return result;
}

template <typename ClassType>
UPError UbxRxmMeasx<ClassType>::ParseRepeatedBlock(const uint8_t* in) {
    if (nullptr == in) {
        return UPError::BadInputParameter;
    }

    repeatedBlock_t block;
    block.gnssId = in[RxmMeasxOffsets::gnssId];
    block.svId = in[RxmMeasxOffsets::svId];
    block.cn0 = in[RxmMeasxOffsets::cn0];
    block.multipath = in[RxmMeasxOffsets::multipath];
    block.pseudoRangeRate = this->template GetValue<int32_t>
    (&in[RxmMeasxOffsets::pseudorangeRate]);
    mParcel.repeated.push_back(block);
    return UPError::Success;
}

//TODO(g.chabukiani): implement validation
//TODO(g.chabukiani): verify time for each constellation and cast it
//to GPS time if needed, thus fix any issues with flags and time sync
template <typename ClassType>
UPError UbxRxmMeasx<ClassType>::ValidateParcel() {
    return UPError::Success;
}

//TODO(g.chabukiani): TOW must be correct value
//should fix it on validation step
template <typename ClassType>
UPError UbxRxmMeasx<ClassType>::GetTowForGnssId(const UbxGnssId inGnssId,
        int64_t& outTow, uint32_t& outState) {
    GnssMs state;

    switch (inGnssId) {
    case UbxGnssId::GPS:
        outTow = mParcel.single.gpsTOW;
        state = GnssMs::STATE_TOW_DECODED;
        break;

    case UbxGnssId::GLONASS:
        outTow = mParcel.single.glonassTOW;
        state = GnssMs::STATE_TOW_DECODED;
        break;

    case UbxGnssId::QZSS:
        outTow = mParcel.single.qzssTOW;
        state = GnssMs::STATE_TOW_DECODED;
        break;

    case UbxGnssId::BEIDOU:
        outTow = mParcel.single.bdsTOW;
        state = GnssMs::STATE_TOW_DECODED;
        break;

    case UbxGnssId::SBAS:
    case UbxGnssId::GALILEO:
        outTow = mParcel.single.gpsTOW; //TODO: find better solution
        state = GnssMs::STATE_TOW_KNOWN;
        break;

    default:
        outTow = 0;
        outState = static_cast<uint32_t>(GnssMs::STATE_UNKNOWN);
        return UPError::BadInputParameter;
    }

    outTow = this->template ScaleUp(outTow,
                                UbxParserCommon<ClassType>::msToNsMultiplier);
    outState = static_cast<uint32_t>(state);
    return UPError::Success;
}

template <typename ClassType>
UPError UbxRxmMeasx<ClassType>::GetTowAccForGnssId(const UbxGnssId inGnssId,
        int64_t& outTowAcc, uint32_t& state) {
    if (0 == mParcel.single.TOWset
        || static_cast<uint32_t>(GnssMs::STATE_UNKNOWN) == state) {
        return UPError::InvalidData;
    }

    uint16_t towAccMs  = 0;

    switch (inGnssId) {
    case UbxGnssId::GPS:
        towAccMs = mParcel.single.gpsTOWacc;
        break;

    case UbxGnssId::GLONASS:
        towAccMs = mParcel.single.glonassTOWacc;
        break;

    case UbxGnssId::QZSS:
        towAccMs = mParcel.single.qzssTOWacc;
        break;

    case UbxGnssId::BEIDOU:
        towAccMs = mParcel.single.bdsTOWacc;
        break;

    case UbxGnssId::SBAS:
    case UbxGnssId::GALILEO:
        towAccMs = mParcel.single.gpsTOWacc;
        break;

    default:
        state = static_cast<uint32_t>(GnssMs::STATE_UNKNOWN);
        return UPError::BadInputParameter;
    }

    int64_t result = this->template ScaleUp(this->template ScaleDown(towAccMs,
                scaleTowAcc), UbxParserCommon<ClassType>::msToNsMultiplier);
    outTowAcc = ((result > 0) ? result : 1);
    return UPError::Success;
}


template <typename ClassType>
GnssConstellationType UbxRxmMeasx<ClassType>::GetConstellationFromGnssId(
    const UbxGnssId inGnssId) {
    switch (inGnssId) {
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

template <typename ClassType>
uint8_t UbxRxmMeasx<ClassType>::GetValidSvidForGnssId(const UbxGnssId inGnssId,
        uint8_t svid) {
    uint8_t resultSvid = svid;

    switch (inGnssId) {
    case UbxGnssId::GPS:
        this->template InRange<uint8_t>(gpsFirst, gpsLast, resultSvid);
        break;

    case UbxGnssId::SBAS:
        this->template InRanges<uint8_t>(sbasOneFirst, sbasOneLast,
                                        sbasTwoFirst, sbasTwoLast, resultSvid);
        break;

    case UbxGnssId::GALILEO:
        this->template InRange<uint8_t>(galileoFirst, galileoLast, resultSvid);
        break;

    case UbxGnssId::QZSS:
        this->template InRange<uint8_t>(qzssFirst, qzssLast, resultSvid);
        break;

    case UbxGnssId::BEIDOU:
        this->template InRange<uint8_t>(bdFirst, bdLast, resultSvid);
        break;

    case UbxGnssId::GLONASS:
        this->template InRanges<uint8_t>(glonassFirst, glonassLast,
                                glonassFcnFirst, glonassFcnLast, resultSvid);
        break;
    }

    return resultSvid;
}

template <typename ClassType>
UbxMsg UbxRxmMeasx<ClassType>::GetMsgType() {
    return mType;
}

template <typename ClassType>
UPError UbxRxmMeasx<ClassType>::GetData(ClassType out) {
    (void)out;
    return UPError::Success;
}

template <typename ClassType>
bool UbxRxmMeasx<ClassType>::IsValid() {
    return mIsValid;
}

template <typename T>
UbxRxmMeasx<T>::~UbxRxmMeasx() {
    //TODO(g.chabukiani): check if we need to clean up
}

#endif // UBXRXMMEASX_H
