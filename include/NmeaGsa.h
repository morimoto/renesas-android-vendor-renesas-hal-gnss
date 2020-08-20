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

#ifndef NMEAGSA_H
#define NMEAGSA_H

#include <log/log.h>

#include "include/NmeaParserCommon.h"

//TODO(g.chabukiani): add doxygen, check all over the project
template <typename T>
class NmeaGsa : public NmeaParserCommon<T> {
public:
    NmeaGsa();
    NmeaGsa(const char* in, const size_t& inLen,
            const NmeaVersion& protocol);
    NmeaGsa(std::string& in, const NmeaVersion& protocol);
    ~NmeaGsa() override {}

    NmeaMsgType GetMsgType() override;
    NmeaVersion GetProtocolVersion() override;
    NPError GetData(T out) override;
    bool IsValid() override;
protected:
    NPError Parse();
    NPError Parse(std::string& in) override;
    NPError ParseCommon(std::vector<std::string>& gsa);
    NPError ValidateParcel();

private:
    typedef struct Parcel {
        size_t gnssId;
        std::vector<int64_t> svList;
    } parcel_t;

    enum GsaOfst : size_t {
        talkerId = 0,
        SvBegin = 3, // Start of repeated block (12 times)
        SvEnd = 15,  // End of repeated block
        SystemId = 18,
    };

    static const NmeaMsgType mType = NmeaMsgType::GSA;
    constexpr static const std::array<size_t, NmeaVersion::AMOUNT>
            mGsaPartsAmount = {18, 19, 19};

    const char* mPayload;
    const size_t mPayloadLen;

    NmeaVersion mCurrentProtocol = NmeaVersion::NMEAv23;
    bool mIsValid = false;
    parcel_t mParcel;

};

template <typename T>
NmeaGsa<T>::NmeaGsa() :
    mPayload(nullptr),
    mPayloadLen(0) {
}

template <typename T>
NmeaGsa<T>::NmeaGsa(std::string& in, const NmeaVersion& protocol) :
    mPayload(in.c_str()),
    mPayloadLen(in.size()),
    mCurrentProtocol(protocol) {
    if (NPError::Success == Parse(in)) {
        mIsValid = true;
    }
}

template <typename T>
NmeaGsa<T>::NmeaGsa(const char* in, const size_t& inLen,
                    const NmeaVersion& protocol) :
    mPayload(in),
    mPayloadLen(inLen),
    mCurrentProtocol(protocol) {
    if (NPError::Success == Parse()) {
        mIsValid = true;
    }
}

template <typename T>
NmeaMsgType NmeaGsa<T>::GetMsgType() {
    return mType;
}

template <typename T>
NmeaVersion NmeaGsa<T>::GetProtocolVersion() {
    return mCurrentProtocol;
}


//TODO(g.chabukiani): implement get data
template <typename T>
NPError NmeaGsa<T>::GetData(T out) {
    (void)out;
    return NPError::Success;
}

template <typename T>
bool NmeaGsa<T>::IsValid() {
    return mIsValid;
}

template <typename T>
NPError NmeaGsa<T>::Parse() {
    if (nullptr == mPayload || mPayloadLen == 0) {
        return NPError::IncompletePacket;
    }

    std::vector<std::string> gsa;
    std::string parcel(mPayload, mPayloadLen);
    this->Split(parcel, gsa);
    return ParseCommon(gsa);
}

template <typename T>
NPError NmeaGsa<T>::ParseCommon(std::vector<std::string>& gsa) {
    if (gsa.size() != mGsaPartsAmount[mCurrentProtocol]) {
        ALOGE("%s - %s - Parts count (%lu) in GSA message is incorrect", LOG_TAG,
              __func__, gsa.size());
        return NPError::IncompletePacket;
    }

    const int base = 10;
    // NMEA defined GNSS System ID available in NMEA 4.10 and later
    if (NmeaVersion::NMEAv41 <= mCurrentProtocol) {
        mParcel.gnssId =
            strtoul(gsa[GsaOfst::SystemId].c_str(), nullptr, base) - 1;
    } else {
        mParcel.gnssId = talkerIdToGnssId(gsa[GsaOfst::talkerId]);
    }
    // Active satellites used for navigation
    for (auto gsaPart = static_cast<size_t>(GsaOfst::SvBegin);
         gsaPart < GsaOfst::SvEnd; gsaPart++) {
        // If less than 12 SVs are used for navigation, the remaining fields are
        // left empty.
        if (gsa[gsaPart].length() > 0) {
            mParcel.svList.push_back(stol(gsa[gsaPart]));
        } else {
            break;
        }
    }

    return ValidateParcel();
}

template <typename T>
NPError NmeaGsa<T>::Parse(std::string& in) {
    NPError status = NPError::Success;
    NP_CH(NmeaParserCommon<T>::Parse(in));
    std::vector<std::string> gsa;
    NP_CH(NmeaParserCommon<T>::Split(in, gsa));
    return ParseCommon(gsa);
}

//TODO(g.chabukiani): implement validation
template <typename T>
NPError NmeaGsa<T>::ValidateParcel() {
    return NPError::Success;
}

#endif // NMEAGSA_H
