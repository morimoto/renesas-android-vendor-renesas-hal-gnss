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
class UbxNavStatus : public UbxParserCommon<T> {
public:
    /*!
     * \brief UbxNavStatus
     * \param in
     * \param size
     */
    UbxNavStatus(const char* in, const size_t& size);

    /*!
     * \brief UbxNavStatus
     */
    ~UbxNavStatus() override;

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
     * \return true if valid, otherwise false
     */
    bool IsValid() override;

protected:
    /*!
     * \brief UbxNavStatus
     */
    UbxNavStatus();

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
    // Using offsets according to the protocol description of UBX-NAV-STATUS
    enum NavStatusOffsets : uint8_t {
        iTowOffset = 0,
        msssOffset = 12,
    };

    typedef struct NavStatus {
        uint32_t iTow;
        uint32_t msss;
    } navStatus_t;

    static const UbxMsg mType = UbxMsg::NAV_STATUS;
    static const size_t mBlockSize = 16;

    const uint8_t* mPayload;
    const size_t mPayloadLen;

    navStatus_t mParcel;
    int64_t mTimeNano = 0;
    bool mIsValid = false;
};


template <typename T>
UbxNavStatus<T>::UbxNavStatus() :
    mPayload(nullptr),
    mPayloadLen(0u) {
}

template <typename T>
UbxNavStatus<T>::UbxNavStatus(const char* in, const size_t& size) :
    mPayload(reinterpret_cast<const uint8_t*>(in)),
    mPayloadLen(size) {
    if (UPError::Success == Parse()) {
        mIsValid = true;
    }
}

template <typename T>
UbxNavStatus<T>::~UbxNavStatus() {}

template <typename T>
UbxMsg UbxNavStatus<T>::GetMsgType() {
    return mType;
}

template <typename T>
UPError UbxNavStatus<T>::GetData(T out) {
    (void) out;
    return UPError::Success;
}

template <typename T>
bool UbxNavStatus<T>::IsValid() {
    return mIsValid;
}

template <typename T>
UPError UbxNavStatus<T>::Parse() {
    if (nullptr == mPayload || mPayloadLen != mBlockSize) {
        ALOGE("[%s, line %d] Payload is not valid", __func__, __LINE__);
        return UPError::IncompletePacket;
    }

    return ParseSingleBlock();
}

template <typename T>
UPError UbxNavStatus<T>::ParseSingleBlock() {
    mParcel.iTow = this->template GetValue<uint32_t>
    (&mPayload[NavStatusOffsets::iTowOffset]);
    mParcel.msss = this->template GetValue<uint32_t>
    (&mPayload[NavStatusOffsets::msssOffset]);
    return ValidateParcel();
}

//TODO(g.chabukiani): implement validation
template <typename T>
UPError UbxNavStatus<T>::ValidateParcel() {
    mTimeNano = this->template ScaleUp(mParcel.msss,
                                       UbxParserCommon<T>::msToNsMultiplier);
    return UPError::Success;
}
