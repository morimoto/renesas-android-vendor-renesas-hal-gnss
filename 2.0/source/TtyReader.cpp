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
#define LOG_TAG "GnssRenesasHalTtyReader"
#include <log/log.h>

#include <memory>
#include <vector>
#include <utility>

#include "include/TtyReader.h"
#include "include/NmeaMsgHandler.h"
#include "include/UbloxMsgHandler.h"
#include "include/MessageQueue.h"

static std::string printLog(const std::vector<char>& in,
                            const bool asHex = true);
static const int maxReadRetries = 5;
static const int tryInterval = 50;

TtyReader::TtyReader(std::shared_ptr<Transport> transport) :
    mTransport(transport),
    mExitThread(false) {
    ALOGV("%s", __func__);
    mEndianType = mTransport->GetEndianType();
}

TtyReader::~TtyReader() {
    Stop();
}

void TtyReader::ReadingLoop() {
    MessageQueue& pipe = MessageQueue::GetInstance();
    int readTry = 0;

    while (!mExitThread) {
        auto parcel = std::make_shared<std::vector<char>>();
        SupportedProtocol protocol = SupportedProtocol::UnknownProtocol;
        RDError result = RDError::InternalError;

        while (!mExitThread && RDError::Success != result) {
            if (TError::Success != mTransport->Read(*parcel)) {
                result = RDError::TransportError;
                std::this_thread::sleep_for(std::chrono::milliseconds(tryInterval));
                ++readTry;
                break;
            }

            result = HandleInput(parcel->back(), protocol);

            if (RDError::Reset == result ) {
                ALOGV("%s, incomplete or lost - %s", __func__, printLog(*parcel).c_str());
                //previous message was incomplete or lost,
                //reset current parcel and continue as from begin
                ResetReader();
                parcel->clear();
            }

            readTry = 0;
        }

        if (RDError::Success == result) {
            if (SupportedProtocol::NMEA0183 == protocol) {
                // send to nmea msg handler
                auto toSend = std::make_shared<nmeaParcel_t>(parcel);
                ALOGV("[%s] nmea %s [size = %zu]",
                      __func__, printLog(*parcel, false).c_str(), parcel->size());
                pipe.Push<std::shared_ptr<nmeaParcel_t>>(toSend);
            } else if (SupportedProtocol::UbxBinaryProtocol == protocol) {
                // send to ublx msg handler
                auto toSend = std::make_shared<ubxParcel_t>
                              (parcel, mUbxPayloadLen.lenUint);
                ALOGV("[%s] ublx %s [size = %zu]",
                      __func__, printLog(*parcel).c_str(), parcel->size());
                pipe.Push<std::shared_ptr<ubxParcel_t>>(toSend);
            } else if (SupportedProtocol::UnknownProtocol != protocol)  {
                // Unexpected!
                ALOGW("[%s] Some unexpected protocol", __func__);
            }
        } else if (maxReadRetries == readTry) {
            ALOGE("%s, Failed to read, %d", __func__, readTry);

            if (nullptr != mDeathNotificationCallback) {
                mDeathNotificationCallback();
            }

            // TODO(g.chabukiani): Reader should be destroyed
            return;
        }

        ResetReader();
    }

    ALOGV("%s, Exit Thread", __func__);
}

static std::string printLog(const std::vector<char>& in, const bool asHex) {
    std::string log;

    if (asHex) {
        char buffer[4096] = {};
        size_t offset = 0;

        for (const auto& x : in) {
            sprintf(&buffer[offset], "0x%02hhX,", x);
            offset += 5;

            if (offset + 5 >= 2048) {
                break;
            }
        }

        log = std::string(buffer, offset);
    } else {
        log = std::string(in.begin(), in.end());
    }

    std::replace(log.begin(), log.end(), mNmeaEndParcelCarriageReturn, ' ');
    std::replace(log.begin(), log.end(), mNmeaEndParcelNewLine, ' ');
    return log;
}

RDError TtyReader::CaptureParcel(const char& ch, SupportedProtocol& protocol) {
    switch (mReaderState) {
    case ReaderState::WAITING: {
        return RDError::Reset;
    }

    case ReaderState::CAPTURING_UBX: {
        ++mReaderUbxParcelOffset;

        if (mReaderUbxParcelOffset == mUbxPayloadLen.lenUint +
            ubxHeaderAndChecksumLen) {
            //Ubx parcel is full.
            ALOGV("%s, Capturing UBLX - SUCCESS", __func__);
            protocol = SupportedProtocol::UbxBinaryProtocol;
            return RDError::Success;
        }

        break;
    }

    case ReaderState::CAPTURING_NMEA: {
        if (mNmeaEndParcelNewLine == ch) {
            ALOGV("%s, Capturing NMEA - SUCCESS", __func__);
            protocol = SupportedProtocol::NMEA0183;
            return RDError::Success;
        }

        break;
    }

    default: {
        ++mReaderUbxParcelOffset;
    }
    }

    return RDError::Capturing;
}

void TtyReader::ResetReader() {
    mReaderState = ReaderState::WAITING;
    mUbxPayloadLen.lenUint = 0u;
    mReaderUbxParcelOffset = 0u;
}

RDError TtyReader::HandleInput(const char& ch, SupportedProtocol& protocol) {
    if (mNmeaBeginParcel == ch &&
        ReaderState::CAPTURING_NMEA == mReaderState) {
        ALOGV("%s, Capturing NMEA - RESET", __func__);
        // Missed end of message CR LF ? Reset the reader
        return RDError::Reset;
    } else if (mNmeaBeginParcel == ch &&
               ReaderState::WAITING == mReaderState) {
        mReaderState = ReaderState::CAPTURING_NMEA;
    } else if (static_cast<char>(UbxSyncByte::UbxSync1) == ch
               && ReaderState::WAITING == mReaderState) {
        mReaderState = ReaderState::WAITING_UBX_SYNC2;
    } else if (static_cast<char>(UbxSyncByte::UbxSync2) == ch
               && ReaderState::WAITING_UBX_SYNC2 == mReaderState) {
        mReaderState = ReaderState::CAPTURING_UBX;
    } else if (ReaderState::CAPTURING_UBX == mReaderState) {
        if (ubxLengthFirstByteOffset == mReaderUbxParcelOffset) {
            mUbxPayloadLen.lenBytes[(mEndianType == Endian::Little ?
                                     mLittleEndianFirsByteOffset :
                                     mBigEndianFistByteOffset)] = ch;
        } else if (ubxLengthSecondByteOffset == mReaderUbxParcelOffset) {
            mUbxPayloadLen.lenBytes[(mEndianType == Endian::Little ?
                                     mLittleEndianSecondByteOffset :
                                     mBigEndianSecondByteOffset)] = ch;
        }
    }

    return CaptureParcel(ch, protocol);
}

RDError TtyReader::SetUpNotificationCallback(cbPtr notificationCb) {
    ALOGV("%s", __func__);

    if (nullptr != notificationCb && nullptr == mDeathNotificationCallback) {
        mDeathNotificationCallback = notificationCb;
        return RDError::Success;
    }

    return RDError::InvalidInput;
}

RDError TtyReader::Start() {
    ALOGV("%s", __func__);

    if (TError::TransportReady != mTransport->Reset()) {
        return RDError::TransportError;
    }

    mExitThread = false;
    mReadingLoop = std::thread{&TtyReader::ReadingLoop, this};
    return RDError::Success;
}

RDError TtyReader::Stop() {
    ALOGV("%s", __func__);
    mExitThread = true;

    if (mReadingLoop.joinable()) {
        mReadingLoop.join();
    }

    return RDError::Success;
}
