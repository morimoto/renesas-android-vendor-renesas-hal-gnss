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
#pragma once

#include "include/NmeaParserCommon.h"
#include <iterator>

using GnssSvFlags = ::android::hardware::gnss::V1_0::IGnssCallback::GnssSvFlags;
using ::android::hardware::gnss::V2_0::GnssConstellationType;
//TODO(g.chabukiani): add doxygen, check all over the project
template <typename T>
class NmeaGsv : public NmeaParserCommon<T> {
public:
    NmeaGsv();
    NmeaGsv(const char* in, const size_t& inLen,
            const NmeaVersion& protocol);
    NmeaGsv(std::string& in, const NmeaVersion& protocol);
    ~NmeaGsv() override {}

    NmeaMsgType GetMsgType() override;
    NmeaVersion GetProtocolVersion() override;
    NPError GetData(T out) override;
    bool IsValid() override;
protected:
    NPError Parse();
    NPError Parse(std::string& in);
    NPError ParseCommon(std::vector<std::string>& gsv);
    NPError ParseSingleBlock(const std::vector<std::string>& in);
    NPError ParseRepeatedBlocks(const std::vector<std::string>& in);
    NPError ParseRepeatedBlock(std::vector<std::string>::const_iterator in);
    NPError ValidateParcel();

private:
    enum GsvOfst {
        gnssId = 0,
        msgAmount = 1,
        curMsgNum = 2,
        numSvInView = 3,

        svid = 0,
        elevation = 1,
        azimuth = 2,
        cn0 = 3,
    };

    typedef struct {
        uint32_t svid;
        uint32_t elevation;
        uint32_t azimuth;
        uint32_t cn0;
        uint32_t orig_svid;
        GnssConstellationType constellation;
    } repeated_t;

    typedef struct {
        NmeaConstellationId gnssId;
        uint8_t msgAmount;
        uint8_t curMsgNum;
        uint32_t numSvInView;
        std::vector<repeated_t> subPart;
        uint8_t svFlag;
        float carrierFrequencyHz;
    } parcel_t;

    void ProcessSvid(repeated_t& block);

    constexpr static const std::array<size_t, NmeaVersion::AMOUNT>
    mGsvMinimalPartsAmount = {7, 8, 8};
    static const NmeaMsgType mType = NmeaMsgType::GSV;
    static const std::vector<std::string>::size_type mSingleBlockFieldNum = 4;
    static const std::vector<std::string>::size_type
                                                    mRepeatedBlockFieldNum = 4;

    static constexpr float L1BandFrequency = 1575.42f;
    static constexpr float B1BandFrequency = 1561.098f;
    static constexpr float L1GlonassBandFrequency = 1602.562f;
    static constexpr float scale = 1000000.0;

    const char* mPayload;
    const size_t mPayloadLen;

    NmeaVersion mCurrentProtocol = NmeaVersion::NMEAv23;
    bool mIsValid = false;
    parcel_t mParcel;
};

template <typename T>
NmeaGsv<T>::NmeaGsv() :
    mPayload(nullptr),
    mPayloadLen(0) {
}

template <typename T>
NmeaGsv<T>::NmeaGsv(std::string& in, const NmeaVersion& protocol) :
    mPayload(in.c_str()),
    mPayloadLen(in.size()),
    mCurrentProtocol(protocol) {
    if (NPError::Success == Parse(in)) {
        mIsValid = true;
    }
}

template <typename T>
NmeaGsv<T>::NmeaGsv(const char* in, const size_t& inLen,
                    const NmeaVersion& protocol) :
    mPayload(in),
    mPayloadLen(inLen),
    mCurrentProtocol(protocol) {
    if (NPError::Success == Parse()) {
        mIsValid = true;
    }
}

template <typename T>
NmeaMsgType NmeaGsv<T>::GetMsgType() {
    return mType;
}

template <typename T>
NmeaVersion NmeaGsv<T>::GetProtocolVersion() {
    return mCurrentProtocol;
}

template <typename T>
NPError NmeaGsv<T>::GetData(T out) {
    (void)out;
    return NPError::Success;
}

template <typename T>
bool NmeaGsv<T>::IsValid() {
    return mIsValid;
}

template <typename T>
NPError NmeaGsv<T>::Parse(std::string& in) {
    if (in.empty()) {
        return NPError::IncompletePacket;
    }
    ALOGV("%s", in.c_str());
    std::vector<std::string> gsv;
    this->Split(in, gsv);
    return ParseCommon(gsv);
}

template <typename T>
NPError NmeaGsv<T>::ParseCommon(std::vector<std::string>& gsv) {
    if (mGsvMinimalPartsAmount[mCurrentProtocol] > gsv.size()) {
        return NPError::IncompletePacket;
    }

    NPError result = ParseSingleBlock(gsv);

    if (NPError::Success != result) {
        return result;
    }

    result = ParseRepeatedBlocks(gsv);

    if (NPError::Success != result) {
        return result;
    }

    return ValidateParcel();
}

template <typename T>
NPError NmeaGsv<T>::Parse() {
    if (nullptr == mPayload || mPayloadLen == 0) {
        return NPError::IncompletePacket;
    }

    std::vector<std::string> gsv;
    std::string parcel(mPayload, mPayloadLen);
    this->Split(parcel, gsv);
    return ParseCommon(gsv);
}

template <typename T>
NPError NmeaGsv<T>::ParseSingleBlock(const std::vector<std::string>& in) {
    const size_t cmpSize = 3;
    const size_t position = 0;
    mParcel.svFlag |= static_cast<uint8_t>(GnssSvFlags::HAS_ALMANAC_DATA)
                    | static_cast<uint8_t>(GnssSvFlags::HAS_CARRIER_FREQUENCY);

    if (in[GsvOfst::gnssId].compare(position, cmpSize, "$GP") == 0) {
        mParcel.gnssId = NmeaConstellationId::GPS_SBAS_QZSS;
        mParcel.carrierFrequencyHz = (L1BandFrequency * scale);
    } else if (in[GsvOfst::gnssId].compare( position, cmpSize, "$GL") == 0) {
        mParcel.gnssId = NmeaConstellationId::GLONASS;
        mParcel.carrierFrequencyHz = (L1GlonassBandFrequency * scale);
    } else if (in[GsvOfst::gnssId].compare( position, cmpSize, "$GA") == 0) {
        mParcel.gnssId = NmeaConstellationId::GALILEO;
        mParcel.carrierFrequencyHz = (L1BandFrequency * scale);
    } else if (in[GsvOfst::gnssId].compare( position, cmpSize, "$GB") == 0) {
        mParcel.gnssId = NmeaConstellationId::BEIDOU;
        mParcel.carrierFrequencyHz = (B1BandFrequency * scale);
    } else if (in[GsvOfst::gnssId].compare( position, cmpSize, "$GN") == 0) {
        mParcel.gnssId = NmeaConstellationId::ANY;
        mParcel.svFlag &=
                ~(static_cast<uint8_t>(GnssSvFlags::HAS_CARRIER_FREQUENCY));
    } else {
        return NPError::BadInputParameter;
    }

    mParcel.msgAmount = std::atoi(in[GsvOfst::msgAmount].c_str());
    mParcel.curMsgNum = std::atoi(in[GsvOfst::curMsgNum].c_str());
    mParcel.numSvInView = std::atoi(in[GsvOfst::numSvInView].c_str());
    return NPError::Success;
}

template <typename T>
NPError NmeaGsv<T>::ParseRepeatedBlocks(const std::vector<std::string>& in) {
    auto result = NPError::UnknownError;
    auto itBegin = in.begin();
    std::advance(itBegin, mSingleBlockFieldNum);
    auto dist = std::distance(itBegin, in.end());

    while (dist >= mRepeatedBlockFieldNum) {
        result = ParseRepeatedBlock(itBegin);

        if (result != NPError::Success) {
            break;
        }

        std::advance(itBegin, mRepeatedBlockFieldNum);
        dist = std::distance(itBegin, in.end());
    }

    return result;
}

template <typename T>
void NmeaGsv<T>::ProcessSvid(repeated_t& block) {
    block.orig_svid = block.svid;
    if (block.svid >= 1 && block.svid <= 32 &&
            mParcel.gnssId == NmeaConstellationId::GPS_SBAS_QZSS) {
        block.constellation = GnssConstellationType::GPS;
    } else if (block.svid >= 1 && block.svid <= 36 &&
                    mParcel.gnssId == NmeaConstellationId::GALILEO) {
        block.constellation = GnssConstellationType::GALILEO;
    } else if (mParcel.gnssId == NmeaConstellationId::GLONASS) {
        block.constellation = GnssConstellationType::GLONASS;
        if (65 <= block.svid && 88 >= block.svid) {
            block.svid -= 64;
        } else {
            block.svid = 93; //TODO
        }
    } else if (mParcel.gnssId == NmeaConstellationId::BEIDOU) {
        block.constellation = GnssConstellationType::BEIDOU;
    } else if ((block.svid >=  33) && (block.svid <=  64)) {
        block.constellation = GnssConstellationType::SBAS;
        block.svid += 87;
    } else if ((block.svid >= 152) && (block.svid <= 158)) {
        block.constellation = GnssConstellationType::SBAS;
        block.svid += 31;
    } else if ((block.svid >= 193) && (block.svid <= 197)) {
        block.constellation = GnssConstellationType::QZSS;
    } else {
        if (mParcel.gnssId != NmeaConstellationId::ANY) {
            ALOGW("Unknown constellation type with Svid = %d", block.svid);
        }
        block.constellation = GnssConstellationType::UNKNOWN;
    }
}

template <typename T>
NPError NmeaGsv<T>::ParseRepeatedBlock(std::vector<std::string>::const_iterator in) {
    repeated_t block;
    block.svid = std::atoi((in + GsvOfst::svid)->c_str());
    ProcessSvid(block);
    block.azimuth = std::atoi((in + GsvOfst::azimuth)->c_str());
    block.elevation = std::atoi((in + GsvOfst::elevation)->c_str());
    block.cn0 = std::atoi((in + GsvOfst::cn0)->c_str());
    mParcel.subPart.push_back(block);
    return NPError::Success;
}
//TODO(g.chabukiani): implement validation
template <typename T>
NPError NmeaGsv<T>::ValidateParcel() {
    return NPError::Success;
}