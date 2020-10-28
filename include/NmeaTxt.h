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

#ifndef NMEATXT_H
#define NMEATXT_H

#include <log/log.h>

#include <NmeaParserCommon.h>

template <typename T>
class NmeaTxt: public NmeaParserCommon<T>{
public:
    NmeaTxt(std::string& in, const NmeaVersion& protocol);
    ~NmeaTxt() override {};

    NmeaMsgType GetMsgType() override;
    NmeaVersion GetProtocolVersion() override;
    NPError GetData(T out) override;
    bool IsValid() override;
    void PrintMsg();
protected:
    NPError Parse();
    NPError Parse(std::string& in);
    NPError ParseMsg(std::vector<std::string>& in);

private:
    enum TxtOfst {
        msgAmount = 1,
        curMsgNum = 2,
        msgType   = 3,
        msgText   = 4
    };

    typedef struct {
        uint8_t msgAmount;
        uint8_t curMsgNum;
        uint8_t msgType;
        std::string text;
    } parcel_t;

    static const size_t mTxtMinimalPartsAmount = 5;
    static const NmeaMsgType mType = NmeaMsgType::TXT;
    const char* mPayload;
    const size_t mPayloadLen;

    NmeaVersion mCurrentProtocol = NmeaVersion::NMEAv23;
    bool mIsValid = false;
    parcel_t mParcel = {};
};

template <typename T>
NmeaTxt<T>::NmeaTxt(std::string& in, const NmeaVersion& protocol) :
    mPayload(in.c_str()),
    mPayloadLen(in.size()),
    mCurrentProtocol(protocol) {
    if (NPError::Success == Parse(in)) {
        mIsValid = true;
    }
}

template <typename T>
NmeaMsgType NmeaTxt<T>::GetMsgType() {
    return mType;
}

template <typename T>
NmeaVersion NmeaTxt<T>::GetProtocolVersion() {
    return mCurrentProtocol;
}

template <typename T>
NPError NmeaTxt<T>::GetData(T out) {
    (void)out;
    return NPError::Success;
}

template <typename T>
bool NmeaTxt<T>::IsValid() {
    return mIsValid;
}

template <typename T>
NPError NmeaTxt<T>::Parse() {
    if (nullptr == mPayload || mPayloadLen == 0) {
        return NPError::IncompletePacket;
    }

    std::vector<std::string> gsv;
    std::string parcel(mPayload, mPayloadLen);
    this->Split(parcel, gsv);
    return ParseMsg(gsv);
}

template <typename T>
NPError NmeaTxt<T>::Parse(std::string& in) {
    if (in.empty()) {
        return NPError::IncompletePacket;
    }
    std::vector<std::string> gsv;
    this->Split(in, gsv);
    return ParseMsg(gsv);
}

template <typename T>
NPError NmeaTxt<T>::ParseMsg(std::vector<std::string>& in) {
    if (mTxtMinimalPartsAmount > in.size()) {
        return NPError::IncompletePacket;
    }

    try {
        mParcel.msgAmount = std::stoi(in[TxtOfst::msgAmount]);
        mParcel.curMsgNum = std::stoi(in[TxtOfst::curMsgNum]);
        mParcel.msgType = std::stoi(in[TxtOfst::msgType]);
    } catch (const std::exception& e) {
        ALOGW("%s", e.what());
        return NPError::InvalidData;
    }

    mParcel.text = in[TxtOfst::msgText];
    return NPError::Success;
}

#endif // NMEATXT_H
