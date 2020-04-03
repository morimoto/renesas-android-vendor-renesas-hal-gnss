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

static void printLog(const std::vector<char>& in);
static const int maxReadRetries = 5;
static const int tryInterval = 50;

TtyReader::TtyReader(Transport& transport) :
    mTransport(transport),
    mExitThread(false) {
    ALOGV("%s", __func__);
    mEndianType = mTransport.GetEndianType();

    if (Endian::Big == mEndianType) {
        mIsLittleEndian = false;
    }
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

        while (RDError::Success != result) {
            if (TError::Success != mTransport.Read(*parcel)) {
                result = RDError::TransportError;
                std::this_thread::sleep_for(std::chrono::milliseconds(tryInterval));
                ++readTry;
                break;
            }

            result = HandleInput(parcel->back(), protocol);

            if (RDError::Reset == result ) {
                ALOGV("%s, incomplete or lost", __func__);
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
                std::string log(parcel->begin(), parcel->end());
                ALOGV("[%s] nmea %s [size = %zu]",
                        __func__, log.c_str(), log.length());
                pipe.Push<std::shared_ptr<nmeaParcel_t>>(toSend);
            } else if (SupportedProtocol::UbxBinaryProtocol == protocol) {
                auto toSend = std::make_shared<ubxParcel_t>
                                        (parcel, mUbxPayloadLen.lenUint);
                //TODO (g.chabukiani): remove printing log
                printLog(*parcel);
                pipe.Push<std::shared_ptr<ubxParcel_t>>(toSend);
                // send to ubx msg handler
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

static void printLog(const std::vector<char>& in) {
    char buffer[4096] = {};
    size_t offset = 0;

    for (const auto& x : in) {
        sprintf(&buffer[offset], "0x%02hhX,", x);
        offset += 5;

        if (offset + 5 >= 2048) {
            break;
        }
    }

    std::string log(buffer, offset);
    ALOGV("[%s] %s [size = %zu]", __func__, log.c_str(), in.size());
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
            ALOGV("%s, New ubx parcel", __func__);
            protocol = SupportedProtocol::UbxBinaryProtocol;
            return RDError::Success;
        }

        break;
    }

    case ReaderState::CAPTURING_NMEA: {
        if (mNmeaEndParcelCarriageReturn == ch ||
            mNmeaEndParcelNewLine == ch) {
            ALOGV("%s, Capturing nmea SUCCESS", __func__);
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
        ALOGV("%s, Capturing nmea RESET", __func__);
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
            mUbxPayloadLen.lenBytes[(mIsLittleEndian ?
                            mLittleEndianFirsByteOffset :
                            mBigEndianFistByteOffset)] = ch;
        } else if (ubxLengthSecondByteOffset == mReaderUbxParcelOffset) {
            mUbxPayloadLen.lenBytes[(mIsLittleEndian ?
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