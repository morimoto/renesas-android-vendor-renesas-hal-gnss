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
#pragma once

#include "include/NmeaParserCommon.h"
using GnssLocationFlags = android::hardware::gnss::V1_0::GnssLocationFlags;

template <typename T>
class NmeaPubx00 : public NmeaParserCommon<T> {
public:
    NmeaPubx00();
    NmeaPubx00(const char* in, const size_t& inLen,
               const NmeaVersion& protocol);
    NmeaPubx00(std::string& in, const NmeaVersion& protocol);
    ~NmeaPubx00() override {}

    NmeaMsgType GetMsgType() override;
    NmeaVersion GetProtocolVersion() override;
    NPError GetData(T out) override;
    bool IsValid() override;
protected:
    NPError Parse();
    NPError Parse(std::string& in);
    NPError ParseCommon(const std::vector<std::string>& in);
    NPError ValidateParcel();
private:
    typedef struct Parcel {
        float horizontalAcc;
        float verticalAcc;
    } parcel_t;

    enum Pubx00Ofst : size_t {
        msgIdOfst = 1,
        horizontalAccuracyOfst = 9,
        verticalAccuracyOfst = 10,
    };

    constexpr static const std::array<size_t, NmeaVersion::AMOUNT>
    mPubx00PartsAmount = {21, 21, 21};
    static const NmeaMsgType mType = NmeaMsgType::PUBX00;
    static const std::string mPubx00MsgId;

    const char* mPayload;
    const size_t mPayloadLen;

    NmeaVersion mCurrentProtocol = NmeaVersion::NMEAv23;
    bool mIsValid = false;
    parcel_t mParcel;
    uint16_t mFlags = 0u;
};

template <typename T>
const std::string NmeaPubx00<T>::mPubx00MsgId = "00";

template <typename T>
NmeaPubx00<T>::NmeaPubx00() :
    mPayload(nullptr),
    mPayloadLen(0) {
}

template <typename T>
NmeaPubx00<T>::NmeaPubx00(std::string& in, const NmeaVersion& protocol) :
    mPayload(in.c_str()),
    mPayloadLen(in.size()),
    mCurrentProtocol(protocol) {
    if (NPError::Success == Parse(in)) {
        mIsValid = true;
    }
}

template <typename T>
NmeaPubx00<T>::NmeaPubx00(const char* in, const size_t& inLen,
                          const NmeaVersion& protocol) :
    mPayload(in),
    mPayloadLen(inLen),
    mCurrentProtocol(protocol) {
    if (NPError::Success == Parse()) {
        mIsValid = true;
    }
}

template <typename T>
NmeaMsgType NmeaPubx00<T>::GetMsgType() {
    return mType;
}

template <typename T>
NmeaVersion NmeaPubx00<T>::GetProtocolVersion() {
    return mCurrentProtocol;
}

//TODO(g.chabukiani): implement get data
template <typename T>
NPError NmeaPubx00<T>::GetData(T out) {
    (void)out;
    return NPError::Success;
}


template <typename T>
bool NmeaPubx00<T>::IsValid() {
    return mIsValid;
}

template <typename T>
NPError NmeaPubx00<T>::ParseCommon(const std::vector<std::string>& pubx) {
    if (pubx.size() != mPubx00PartsAmount[mCurrentProtocol]) {
        return NPError::IncompletePacket;
    }

    if (mPubx00MsgId != pubx.at(msgIdOfst)) {
        return NPError::InvalidData;
    }

    if (pubx[horizontalAccuracyOfst].length() > 0 ) {
        mParcel.horizontalAcc = std::stof(pubx[horizontalAccuracyOfst]);
        mFlags |= GnssLocationFlags::HAS_HORIZONTAL_ACCURACY;
    }

    if (pubx[verticalAccuracyOfst].length() > 0 ) {
        mParcel.verticalAcc = std::stof(pubx[verticalAccuracyOfst]);
        mFlags |= GnssLocationFlags::HAS_VERTICAL_ACCURACY;
    }

    return ValidateParcel();
}

template <typename T>
NPError NmeaPubx00<T>::Parse(std::string& in) {
    if (in.empty()) {
        return NPError::IncompletePacket;
    }

    std::vector<std::string> pubx;
    this->Split(in, pubx);
    return ParseCommon(pubx);
}

template <typename T>
NPError NmeaPubx00<T>::Parse() {
    if (nullptr == mPayload || mPayloadLen == 0) {
        return NPError::IncompletePacket;
    }

    std::vector<std::string> pubx;
    std::string parcel(mPayload, mPayloadLen);
    this->Split(parcel, pubx);
    return ParseCommon(pubx);
}

//TODO(g.chabukiani): implement validation
template <typename T>
NPError NmeaPubx00<T>::ValidateParcel() {
    return NPError::Success;
}
