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

using GnssCF =
    android::hardware::gnss::V2_0::IGnssMeasurementCallback::GnssClockFlags;

template <typename ClassType>
class UbxNavTimeGps : public UbxParserCommon<ClassType> {
public:
    UbxNavTimeGps(const char* in, const size_t& inLen);
    ~UbxNavTimeGps() override;

    UbxMsg GetMsgType() override;
    UPError GetData(ClassType out) override;
    bool IsValid() override;
protected:
    UbxNavTimeGps();
    UPError Parse();
    UPError ParseSingleBlock();
    UPError CheckFlags();
    UPError SetTimeNano();
    UPError ValidateParcel();
    bool IsValidFlag(const uint8_t& flags, const uint8_t& expFlag);

private:
    enum NavTimeGpsOffsets : uint8_t {
        iTowOffset = 0,
        fTowOffset = 4,
        weekOffset = 8,
        leapSOffset = 10,
        validOffset = 11,
        tAccOffset = 12,
    };

    typedef struct SingleBlock {
        uint32_t iTow; // GPS time of week of the navigation epoch. (ms)
        int32_t fTow; // Fractional part of iTOW (ns)
        int16_t week; // GPS week number of the navigation epoch
        int8_t leapS;
        uint8_t valid;
        uint32_t tAcc; // Time Accuracy Estimate (ns)
    } singleBlock_t;

    const uint8_t* mPayload;
    const size_t mPayloadLen;

    static const UbxMsg mType = UbxMsg::NAV_TIME_GPS;
    static const size_t mBlockSize = 16;
    static const int64_t fullWeekMs = 604800000; // ms in week
    const int64_t defaultRtcTime = 1;

    uint16_t      mClockFlags = 0;
    singleBlock_t mParcel = {};
    int64_t mTimeNano = 0;
    bool mIsValid = false;
};

template <typename ClassType>
UbxNavTimeGps<ClassType>::UbxNavTimeGps() :
    mPayload(nullptr),
    mPayloadLen(0) {}


template <typename ClassType>
UbxNavTimeGps<ClassType>::UbxNavTimeGps(const char* in,
                                        const size_t& inLen) :
    mPayload(reinterpret_cast<const uint8_t*>(in)),
    mPayloadLen(inLen) {
    if (UPError::Success == Parse()) {
        mIsValid = true;
    }
}

template <typename ClassType>
UPError UbxNavTimeGps<ClassType>::Parse() {
    if (nullptr == mPayload || mPayloadLen != mBlockSize) {
        return UPError::IncompletePacket;
    }

    return ParseSingleBlock();
}

template <typename ClassType>
UPError UbxNavTimeGps<ClassType>::ParseSingleBlock() {
    mParcel.iTow = this->template GetValue<uint32_t>
    (&mPayload[NavTimeGpsOffsets::iTowOffset]);
    mParcel.fTow = this->template GetValue<int32_t>
    (&mPayload[NavTimeGpsOffsets::fTowOffset]);
    mParcel.week = this->template GetValue<int16_t>
    (&mPayload[NavTimeGpsOffsets::weekOffset]);
    mParcel.leapS = static_cast<int8_t>(
                        mPayload[NavTimeGpsOffsets::leapSOffset]);
    mParcel.valid = mPayload[NavTimeGpsOffsets::validOffset];
    mParcel.tAcc = this->template GetValue<uint32_t>
    (&mPayload[NavTimeGpsOffsets::tAccOffset]);
    return ValidateParcel();
}

template <typename ClassType>
bool UbxNavTimeGps<ClassType>::IsValidFlag(const uint8_t& flags,
        const uint8_t& expFlag) {
    uint8_t val = (flags & expFlag);

    if (expFlag == val) {
        return true;
    }

    return false;
}

template <typename ClassType>
UPError UbxNavTimeGps<ClassType>::CheckFlags() {
    const uint8_t validTowMask = 0x01;
    const uint8_t validWeekMask = 0x02;
    const uint8_t validLeapMask = 0x04;
    bool validTow = IsValidFlag(mParcel.valid, validTowMask);
    bool validWeek = IsValidFlag(mParcel.valid, validWeekMask);

    if (IsValidFlag(mParcel.valid, validLeapMask)) {
        mClockFlags |= static_cast<uint16_t>(GnssCF::HAS_LEAP_SECOND);
    }

    if (validWeek && validTow) {
        mClockFlags |= static_cast<uint16_t>
                        (GnssCF::HAS_TIME_UNCERTAINTY | GnssCF::HAS_FULL_BIAS);
        return UPError::Success;
    }

    return UPError::InvalidData;
}

template <typename ClassType>
UPError UbxNavTimeGps<ClassType>::SetTimeNano() {
    int64_t tmpGpsTimeNs = this->template ScaleUp(mParcel.week * fullWeekMs,
                           UbxParserCommon<ClassType>::msToNsMultiplier);
    int64_t tmpTowNs = this->template ScaleUp(mParcel.iTow,
                UbxParserCommon<ClassType>::msToNsMultiplier) + mParcel.fTow;
    mTimeNano = tmpGpsTimeNs + tmpTowNs;
    return UPError::Success;
}

template <typename ClassType>
UbxMsg UbxNavTimeGps<ClassType>::GetMsgType() {
    return mType;
}

template <typename ClassType>
UPError UbxNavTimeGps<ClassType>::GetData(ClassType out) {
    (void)out;
    return UPError::Success;
}

template <typename ClassType>
bool UbxNavTimeGps<ClassType>::IsValid() {
    return mIsValid;
}

template <typename ClassType>
UPError UbxNavTimeGps<ClassType>::ValidateParcel() {
    UPError result = UPError::UnknownError;

    do {
        result = CheckFlags();

        if (UPError::Success != result) {
            break;
        }

        result = SetTimeNano();

        if (UPError::Success != result) {
            break;
        }

        //TODO(g.chabukiani): imlement time validation here
    } while (0);

    return result;
}

template <typename T>
UbxNavTimeGps<T>::~UbxNavTimeGps() {
    //TODO(g.chabukiani): check if we need to clean up
}
