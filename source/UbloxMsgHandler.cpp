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

#define LOG_NDEBUG 1
#define LOG_TAG "GnssRenesasHalUbloxMsgHandler"

#include "include/UbloxMsgHandler.h"

#include "include/MessageQueue.h"
#include "include/UbxAckNack.h"
#include "include/UbxMonVer.h"
#include "include/UbxNavClock.h"
#include "include/UbxNavPvt.h"
#include "include/UbxNavStatus.h"
#include "include/UbxNavTimeGps.h"
#include "include/UbxRxmMeasx.h"

using GnssData =
    android::hardware::gnss::V2_1::IGnssMeasurementCallback::GnssData;

UbxMsgHandler::UbxMsgHandler(const UbxProtocolVersion& protocol) :
    mExitThread(false),
    mProtocol(protocol),
    mPipe(MessageQueue::GetInstance()),
    mWaitCv(MessageQueue::GetInstance().GetConditionVariable
                <std::shared_ptr<ubxParcel_t>>()) {
    ALOGV("%s", __func__);
}

UbxMsgHandler::UbxMsgHandler() :
    mExitThread(false),
    mPipe(MessageQueue::GetInstance()),
    mWaitCv(MessageQueue::GetInstance().GetConditionVariable
                <std::shared_ptr<ubxParcel_t>>()) {
    ALOGV("%s", __func__);
}

UbxMsgHandler::~UbxMsgHandler() {
    StopProcessing();
}

UMHError UbxMsgHandler::StartProcessing() {
    ALOGV("%s", __func__);

    if (!mProcessingThread.joinable()) {
        mProcessingThread = std::thread(&UbxMsgHandler::ProcessingLoop, this);
        return UMHError::Success;
    }

    return UMHError::InternalError;
}

UMHError UbxMsgHandler::StopProcessing() {
    ALOGV("%s", __func__);
    mExitThread = true;

    if (mProcessingThread.joinable()) {
        mProcessingThread.join();
        return UMHError::Success;
    }

    return UMHError::InternalError;
}

//TODO (g.chabukiani): use real types for each parcel
typedef int outType;

UMHError UbxMsgHandler::SelectParser() {
    ALOGV("%s", __func__);
    MessageQueue& instance = MessageQueue::GetInstance();

    if (UbxClass::ACK == mParcel.rxClass && UbxId::ACK == mParcel.rxId) {
        AckQueueType sp = std::make_shared<UbxAckNack<AckOutType>>
                        (mParcel.msgPayload,
                        mParcel.lenght,
                        UbxMsg::ACK_ACK);
        instance.Push<AckQueueType>(sp);
    } else if (UbxClass::ACK == mParcel.rxClass
                && UbxId::NACK == mParcel.rxId) {
        ALOGV("%s, nack", __func__);
        AckQueueType sp = std::make_shared<UbxAckNack<AckOutType>>
                        (mParcel.msgPayload,
                        mParcel.lenght,
                        UbxMsg::ACK_NACK);
        instance.Push<AckQueueType>(sp);
    } else if (UbxClass::NAV == mParcel.rxClass
                    && UbxId::TIMEGPS == mParcel.rxId) {
        auto sp = std::make_shared<UbxNavTimeGps<GnssData*>>(mParcel.msgPayload,
                  mParcel.lenght);
        instance.Push<std::shared_ptr<IUbxParser<GnssData*>>>(sp);
    } else if (UbxClass::NAV == mParcel.rxClass
                    && UbxId::CLOCK == mParcel.rxId) {
        auto sp = std::make_shared<UbxNavClock<GnssData*>>(mParcel.msgPayload,
                  mParcel.lenght);
        instance.Push<std::shared_ptr<IUbxParser<GnssData*>>>(sp);
    } else if (UbxClass::NAV == mParcel.rxClass
                    && UbxId::STATUS == mParcel.rxId) {
        auto sp = std::make_shared<UbxNavStatus<GnssData*>>(mParcel.msgPayload,
                  mParcel.lenght);
        instance.Push<std::shared_ptr<IUbxParser<GnssData*>>>(sp);
    } else if (UbxClass::NAV == mParcel.rxClass
                    && UbxId::PVT == mParcel.rxId) {
        auto sp = std::make_shared<UbxNavPvt<outType>>(mParcel.msgPayload,
                  mParcel.lenght);
        instance.Push<std::shared_ptr<IUbxParser<outType>>>(sp);
    } else if (UbxClass::RXM == mParcel.rxClass
                    && UbxId::MEASX == mParcel.rxId) {
        auto sp = std::make_shared<UbxRxmMeasx<GnssData*>>(mParcel.msgPayload,
                  mParcel.lenght);
        instance.Push<std::shared_ptr<IUbxParser<GnssData*>>>(sp);
    } else if (UbxClass::MON == mParcel.rxClass
                    && UbxId::VER == mParcel.rxId) {
        MonVerQueueType sp = std::make_shared<UbxMonVer<monVerOut>>
                            (mParcel.msgPayload,
                            mParcel.lenght);
        instance.Push<MonVerQueueType>(sp);
    } else {
        //TODO(g.chabukiani): handle incorrect messages
        return UMHError::InternalError;
    }

    return UMHError::Success;
}

UMHError UbxMsgHandler::VerifyCheckSum(const std::vector<char>& parcel,
                                       const uint16_t& len) {
    auto begin = parcel.begin() + classOffset;
    auto end = parcel.end() - endLeftShifCrc;
    std::for_each(begin, end, [this] (const char& ch) {
        mParcel.rxChecksumA += static_cast<uint8_t>(ch);
        mParcel.rxChecksumB += mParcel.rxChecksumA;
    });

    if (parcel.at(payloadOffset + len) == mParcel.rxChecksumA
        && parcel.at(payloadOffset + len + checksum2Offset)
                                    == mParcel.rxChecksumB) {
        return UMHError::Success;
    }

    ALOGV("%s, incorrect checksum", __func__);
    return UMHError::InternalError;
}

void UbxMsgHandler::ProcessingLoop() {
    ALOGV("%s", __func__);
    const size_t timeout = 5000;

    while (!mExitThread) {
        ALOGV("%s, !mExitThread", __func__);

        if (mPipe.Empty<std::shared_ptr<ubxParcel_t>>()) {
            std::unique_lock<std::mutex> lock(mLock);
            ALOGV("%s, wait", __func__);
            mWaitCv.wait_for(lock, std::chrono::milliseconds{timeout},
                [this] {return !mPipe.Empty<std::shared_ptr<ubxParcel_t>>();});
        }

        ALOGV("%s, pop", __PRETTY_FUNCTION__);
        auto parcel = mPipe.Pop<std::shared_ptr<ubxParcel_t>>();

        if  (nullptr != parcel) {
            if (UMHError::Success == Process(*parcel)) {
                SelectParser();
            }
        }
    }

    ALOGV("%s, exit thread", __func__);
}

UMHError UbxMsgHandler::Process(const ubxParcel_t& ubxParcel) {
    ALOGV("%s", __func__);

    if (ubxParcel.sp->size() <
                ubxParcel.mPayloadLen + ubxHeaderAndChecksumLen) {
        ALOGV("%s, Wrong size of processed parcel", __PRETTY_FUNCTION__);
        return UMHError::FailedToProcess;
    }

    Reset();

    if (auto res = VerifyCheckSum(*ubxParcel.sp, ubxParcel.mPayloadLen);
        UMHError::Success != res) {
        ALOGV("%s, Wrong checksum", __func__);
        return res;
    }

    mParcel.rxClass = static_cast<UbxClass>(ubxParcel.sp->at(classOffset));
    mParcel.rxId = static_cast<UbxId>(ubxParcel.sp->at(idOffset));
    mParcel.lenght = ubxParcel.mPayloadLen;
    mParcel.msgPayload = (ubxParcel.sp->begin() + payloadOffset).base();
    return UMHError::Success;
}

void UbxMsgHandler::Reset() {
    ALOGV("%s", __func__);
    mParcel.rxClass = UbxClass::UNKNOWN;
    mParcel.rxId = UbxId::UNKNOWN;
    mParcel.rxChecksumA = 0u;
    mParcel.rxChecksumB = 0u;
    mParcel.lenght = 0u;
    mParcel.msgPayload = nullptr;
}

UMHError UbxMsgHandler::UpdateProtocolVerion(const UbxProtocolVersion&
        protocol) {
    (void)protocol;
    return UMHError::InternalError;
}
