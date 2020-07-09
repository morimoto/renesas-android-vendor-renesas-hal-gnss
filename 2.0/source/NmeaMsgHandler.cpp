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
#define LOG_TAG "GnssRenesasNmeaMsgHandler"
#include <log/log.h>

#include "include/NmeaMsgHandler.h"
#include "include/NmeaGga.h"
#include "include/NmeaGsa.h"
#include "include/NmeaGsv.h"
#include "include/NmeaRmc.h"
#include "include/NmeaPubx00.h"
#include "include/NmeaTxt.h"

NMHError ValidateParcel(std::string& in);

NmeaMsgHandler::NmeaMsgHandler(const NmeaVersion& protocol) :
    mExitThread(false),
    mProtocol(protocol),
    mPipe(MessageQueue::GetInstance()),
    mWaitCv(MessageQueue::GetInstance().GetConditionVariable
        <std::shared_ptr<nmeaParcel_t>>()) {
}

NmeaMsgHandler::NmeaMsgHandler() :
    mExitThread(false),
    mPipe(MessageQueue::GetInstance()),
    mWaitCv(MessageQueue::GetInstance().GetConditionVariable
        <std::shared_ptr<nmeaParcel_t>>()) {
}

NmeaMsgHandler::~NmeaMsgHandler() {
    StopProcessing();
}

NMHError NmeaMsgHandler::StartProcessing() {
    if (!mProcessingThread.joinable()) {
        mExitThread = false;
        mProcessingThread = std::thread(&NmeaMsgHandler::ProcessingLoop, this);
        return NMHError::Success;
    }

    return NMHError::InternalError;
}

NMHError NmeaMsgHandler::StopProcessing() {
    mExitThread = true;

    if (mProcessingThread.joinable()) {
        mProcessingThread.join();
        return NMHError::Success;
    }

    return NMHError::InternalError;
}

NMHError NmeaMsgHandler::SelectParser(std::string& in) {
    if (in.empty()) {
        return NMHError::InternalError;
    }

    if (std::string::npos != in.find(ggaHeader)) {
        auto sp = std::make_shared<NmeaGga<LocationExtraInfoOutType>>
                                                            (in, mProtocol);
        mPipe.Push<LocationExtraInfoQueueType>(sp);
    } else if (std::string::npos != in.find(gsaHeader)) {
        auto sp = std::make_shared<NmeaGsa<SvInfoFixOutType>>(in, mProtocol);
        mPipe.Push<SvInfoFixQueueType>(sp);
    } else if (std::string::npos != in.find(gsvHeader)) {
        auto sp = std::make_shared<NmeaGsv<SvInfoOutType>>(in, mProtocol);
        mPipe.Push<SvInfoQueueType>(sp);
    } else if (std::string::npos != in.find(rmcHeader)) {
        auto sp = std::make_shared<NmeaRmc<LocationOutType>>(in, mProtocol);
        mPipe.Push<LocationQueueType>(sp);
    } else if (std::string::npos != in.find(pubx00Header)) {
        auto sp = std::make_shared<NmeaPubx00<LocationExtraInfoOutType>>
                                                            (in, mProtocol);
        mPipe.Push<LocationExtraInfoQueueType>(sp);
    } else if (std::string::npos != in.find(txtHeader)) {
        auto sp = std::make_shared<NmeaTxt<std::string&>>(in, mProtocol);
        sp->PrintMsg();
    } else {
        //error, should not occur
        return NMHError::InternalError;
    }

    return NMHError::Success;
}

void NmeaMsgHandler::ProcessingLoop() {
    while (!mExitThread) {
        if (mPipe.Empty<std::shared_ptr<nmeaParcel_t>>()) {
            std::unique_lock<std::mutex> lock(mLock);
            mWaitCv.wait(lock, [this]
                    {return !mPipe.Empty<std::shared_ptr<nmeaParcel_t>>();});
        }

        auto parcel = mPipe.Pop<std::shared_ptr<nmeaParcel_t>>();

        std::string curParcel(parcel->sp->begin(), parcel->sp->end());
        if (NMHError::Success == ValidateParcel(curParcel)) {
            if (NMHError::Success != SelectParser(curParcel)) {
                // Error should be processed correctly
            }
        }
    }
}

NMHError ValidateParcel(std::string& in) {
    const std::string::size_type secondCharacterPos = 1;
    const std::string::size_type shiftPos           = 1;
    const std::string::size_type minimumPacketSize  = 8;
    const std::string::size_type parcelSize         = in.size();
    const std::string::size_type checkSumCharPosR   = 5;
    const char                   checkSumChar       = '*';
    const int                    hexBase            = 16;

    if (in.size() < minimumPacketSize || in.front() != mNmeaBeginParcel ||
        in.back() != mNmeaEndParcelNewLine ||
        in[parcelSize - checkSumCharPosR] != checkSumChar) {
        ALOGE("%s - Incorrect message structure - %s", __func__, in.c_str());
        return NMHError::FailedToProcess;
    }

    unsigned short checkSum = in[secondCharacterPos];
    for (auto character : in.substr(
             secondCharacterPos + shiftPos,
             parcelSize - checkSumCharPosR - secondCharacterPos - shiftPos)) {
        checkSum = checkSum ^ character;
    }

    if (checkSum !=
        std::stoul(in.substr(parcelSize - checkSumCharPosR + shiftPos,
                             secondCharacterPos + shiftPos),
                   nullptr, hexBase)) {
        ALOGE("%s - Bad checksum - %s", __func__, in.c_str());
        return NMHError::BadCrc;
    }

    return NMHError::Success;
}

NMHError NmeaMsgHandler::UpdateProtocolVersion(const NmeaVersion& protocol) {
    mProtocol = protocol;
    return NMHError::InternalError;
}
