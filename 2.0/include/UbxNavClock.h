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

// Using offsets according to the protocol description of UBX-NAV-CLOCK
enum NavClockOffsets : uint8_t {
    iTow = 0,
    clockBias = 4,
    clockDrift = 8,
    timeAccuracy = 12,
    freqAccuracyEstimate = 16,
};

template <typename T>
class UbxNavClock : public UbxParserCommon<T> {
public:
    /*!
     * \brief UbxNavClock
     * \param in
     * \param size
     */
    UbxNavClock(const char* in, const size_t& size);

    /*!
     * \brief ~UbxNavClock
     */
    ~UbxNavClock() override;

    /*!
     * \brief GetMsgType
     * \return
     */
    UbxMsg GetMsgType() override;

    /*!
     * \brief GetData
     * \param out
     * \return
     */
    UPError GetData(T out) override;

    /*!
     * \brief IsValid
     * \return
     */
    bool IsValid() override;

protected:
    /*!
     * \brief UbxNavClock
     */
    UbxNavClock();

    /*!
     * \brief Parse
     * \return
     */
    UPError Parse();

    /*!
     * \brief ParseSingleBlock
     * \return
     */
    UPError ParseSingleBlock();

    /*!
     * \brief ValidateParcel
     * \return
     */
    UPError ValidateParcel();

private:
    typedef struct NavClock {
        uint32_t iTow;
        int32_t clockBias;
        int32_t clockDrift;
        uint32_t timeAccuracy;
        uint32_t freqAccuracyEstimate;
    } navClock_t;

    static const UbxMsg mType = UbxMsg::NAV_CLOCK;
    static const size_t mBlockSize = 20;

    const uint8_t* mPayload;
    const size_t mPayloadLen;

    bool mIsValid = false;
    navClock_t mParcel;
};

template <typename T>
UbxNavClock<T>::UbxNavClock() :
    mPayload(nullptr),
    mPayloadLen(0u) {
}

template <typename T>
UbxNavClock<T>::UbxNavClock(const char* in, const size_t& size) :
    mPayload(reinterpret_cast<const uint8_t*>(in)),
    mPayloadLen(size) {
    if (UPError::Success == Parse()) {
        mIsValid = true;
    }
}

template <typename T>
UbxMsg UbxNavClock<T>::GetMsgType() {
    return mType;
}

template <typename T>
UPError UbxNavClock<T>::GetData(T out) {
    (void) out;
    return UPError::Success;
}

//TODO(g.chabukiani): implement validation
template <typename T>
UPError UbxNavClock<T>::ValidateParcel() {
    return UPError::Success;
}

template <typename T>
bool UbxNavClock<T>::IsValid() {
    return mIsValid;
}

template <typename T>
UPError UbxNavClock<T>::ParseSingleBlock() {
    mParcel.iTow = this->template GetValue<uint32_t>
    (&mPayload[NavClockOffsets::iTow]);
    mParcel.clockBias = this->template GetValue<int32_t>
    (&mPayload[NavClockOffsets::clockBias]);
    mParcel.clockDrift = this->template GetValue<int32_t>
    (&mPayload[NavClockOffsets::clockDrift]);
    mParcel.timeAccuracy = this->template GetValue<uint32_t>
    (&mPayload[NavClockOffsets::timeAccuracy]);
    mParcel.freqAccuracyEstimate = this->template GetValue<uint32_t>
    (&mPayload[NavClockOffsets::freqAccuracyEstimate]);
    return ValidateParcel();
}

template <typename T>
UPError UbxNavClock<T>::Parse() {
    if (nullptr == mPayload || mPayloadLen != mBlockSize) {
        ALOGE("[%s, line %d] Payload is not valid", __func__, __LINE__);
        return UPError::IncompletePacket;
    }

    return ParseSingleBlock();
}

template <typename T>
UbxNavClock<T>::~UbxNavClock() {
    //TODO(g.chabukiani): check if we need to clean up
}
