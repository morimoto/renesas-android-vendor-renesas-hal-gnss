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
class UbxAckNack : public UbxParserCommon<T> {
public:
    /*!
     * \brief UbxAckNack
     * \param in
     * \param size
     */
    UbxAckNack(const char* in, const size_t& size, const UbxMsg& msgType);

    /*!
     * \brief UbxAckNack
     */
    ~UbxAckNack() override;

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
    UPError GetData([[maybe_unused]]T out) override;

    /*!
     * \brief IsValid
     * \return true if valid, otherwise false
     */
    bool IsValid() override;

protected:
    /*!
     * \brief UbxAckNack
     */
    UbxAckNack();

    /*!
     * \brief Parse
     * \return
     */
    UPError Parse();

private:
    // Using offsets according to the protocol description of UBX-NAV-STATUS
    enum AckNackOffst : uint8_t {
        cl = 0,
        id = 1,
    };

    typedef struct NavStatus {
        uint8_t cl;
        uint8_t id;
    } ack_t;

    static const size_t mBlockSize = 2;

    const uint8_t* mPayload;
    const size_t mPayloadLen;
    const UbxMsg mType;

    ack_t mParcel;
    bool mIsValid = false;
};


template <typename T>
UbxAckNack<T>::UbxAckNack() :
    mPayload(nullptr),
    mPayloadLen(0u) {
}

template <typename T>
UbxAckNack<T>::UbxAckNack(const char* in, const size_t& size,
                          const UbxMsg& msgType) :
    mPayload(reinterpret_cast<const uint8_t*>(in)),
    mPayloadLen(size),
    mType(msgType) {
    if (UPError::Success == Parse()) {
        mIsValid = true;
    }
}

template <typename T>
UbxAckNack<T>::~UbxAckNack() {}

template <typename T>
UbxMsg UbxAckNack<T>::GetMsgType() {
    return mType;
}

template <typename T>
UPError UbxAckNack<T>::GetData([[maybe_unused]]T out) {
    if (!mIsValid) {
        return UPError::InvalidData;
    }

    return UPError::Success;
}

template <typename T>
bool UbxAckNack<T>::IsValid() {
    return mIsValid;
}

template <typename T>
UPError UbxAckNack<T>::Parse() {
    if (nullptr == mPayload || mPayloadLen != mBlockSize) {
        return UPError::IncompletePacket;
    }

    mParcel.cl = mPayload[AckNackOffst::cl];
    mParcel.id = mPayload[AckNackOffst::id];
    return UPError::Success;
}
