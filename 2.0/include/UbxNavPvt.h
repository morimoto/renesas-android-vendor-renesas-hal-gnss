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

#include "include/UbxParserCommon.h"

template <typename T>
class UbxNavPvt : public UbxParserCommon<T> {
public:
    UbxNavPvt(const char* in, const size_t& inLen);
    ~UbxNavPvt() override {}

    UbxMsg GetMsgType() override;
    UPError GetData(T out) override;
    bool IsValid() override;

protected:
    UbxNavPvt();
    UPError Parse();
    UPError ParseSingleBlock();
    UPError ValidateParcel();


private:
    typedef struct NavPvt {
        uint8_t fixType;
        uint8_t flag1;
        uint8_t flag2;
        uint8_t numSvs;
        int32_t lon;
        int32_t lat;
        int32_t heightMSL;
        uint32_t horizontalAcc;
        uint32_t verticalAcc;
        int32_t groundSpeed;
        int32_t headingOfMotion;
        uint32_t speedAcc;
        uint32_t headingAcc;
    } navPvt_t;

    enum NavPvtOffset : uint8_t {
        fixTypeOffset = 20,
        flag1Offset = 21,
        flag2Offset = 22,
        numSvsOffset = 23,
        lonOffset = 24,
        latOffset = 28,
        heightMSLOffset = 36,
        horizontalAccOffset = 40,
        verticalAccOffset = 44,
        groundSpeedOffset = 60,
        headingOfMotionOffset = 64,
        speedAccOffset = 68,
        headingAccOffset = 72,
    };

    static const size_t mBlockSize = 84;
    static const UbxMsg mType = UbxMsg::NAV_PVT;

    const uint8_t* mPayload;
    const size_t mPayloadLen;

    bool mIsValid = false;
    navPvt_t mParcel;
};

template <typename T>
UbxNavPvt<T>::UbxNavPvt(const char* in, const size_t& inLen) :
    mPayload(reinterpret_cast<const uint8_t*>(in)),
    mPayloadLen(inLen) {
}

template <typename T>
UbxNavPvt<T>::UbxNavPvt() :
    mPayload(nullptr),
    mPayloadLen(0) {
}

template <typename T>
UbxMsg UbxNavPvt<T>::GetMsgType() {
    return mType;
}

template <typename T>
UPError UbxNavPvt<T>::GetData(T out) {
    (void)out;
    return UPError::Success;
}

template <typename T>
bool UbxNavPvt<T>::IsValid() {
    return mIsValid;
}

template <typename T>
UPError UbxNavPvt<T>::Parse() {
    if (nullptr == mPayload || mPayloadLen >= mBlockSize) {
        return UPError::IncompletePacket;
    }

    return ParseSingleBlock();
}

template <typename T>
UPError UbxNavPvt<T>::ParseSingleBlock() {
    mParcel.fixType = mPayload[fixTypeOffset];
    mParcel.flag1 = mPayload[flag1Offset];
    mParcel.flag2 = mPayload[flag2Offset];
    mParcel.numSvs = mPayload[numSvsOffset];
    mParcel.lon = this->template GetValue<int32_t>(&mPayload[lonOffset]);
    mParcel.lat = this->template GetValue<int32_t>(&mPayload[latOffset]);
    mParcel.heightMSL = this->template GetValue<int32_t>
    (&mPayload[heightMSLOffset]);
    mParcel.horizontalAcc = this->template GetValue<uint32_t>
    (&mPayload[horizontalAccOffset]);
    mParcel.verticalAcc = this->template GetValue<uint32_t>
    (&mPayload[verticalAccOffset]);
    mParcel.groundSpeed = this->template GetValue<int32_t>
    (&mPayload[groundSpeedOffset]);
    mParcel.headingOfMotion = this->template GetValue<int32_t>
    (&mPayload[headingOfMotionOffset]);
    mParcel.speedAcc = this->template GetValue<uint32_t>(
                                                &mPayload[speedAccOffset]);
    mParcel.headingAcc = this->template GetValue<uint32_t>
    (&mPayload[headingAccOffset]);
    return ValidateParcel();
}

//TODO: implement validation of parcel
template <typename T>
UPError UbxNavPvt<T>::ValidateParcel() {
    return UPError::Success;
}
