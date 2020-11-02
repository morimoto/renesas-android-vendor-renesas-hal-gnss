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

#include <TtyReader.h>

#include <MessageQueue.h>
#include <NmeaMsgHandler.h>
#include <UbloxMsgHandler.h>

static std::string printLog(const std::vector<char>& in,
                            const bool asHex = true);
static const int maxReadRetries = 5;
static const int tryInterval = 50;

TtyReader::TtyReader(std::shared_ptr<Transport> transport):
        mTransport(transport),
        mEndianType(mTransport->GetEndianType()),
        mFirstByteOffset(mEndianType == Endian::Little ? 0u : 1u),
        mSecondByteOffset(mEndianType == Endian::Little ? 1u : 0u),
        mExitThread(false) {
    ALOGV("%s", __func__);
}

TtyReader::~TtyReader() {
    Stop();
}

void TtyReader::SendToMsgHandler(const std::shared_ptr<std::vector<char>>& parcel,
                                 const SupportedProtocol& protocol) {
    MessageQueue& pipe = MessageQueue::GetInstance();

    switch (protocol) {
        case SupportedProtocol::NMEA0183: {
            // send to nmea msg handler
            auto toSend = std::make_shared<nmeaParcel_t>(parcel);
            ALOGV("[%s] nmea %s [size = %zu]",
                  __func__, printLog(*parcel, false).c_str(), parcel->size());
            pipe.Push<std::shared_ptr<nmeaParcel_t>>(toSend);
            break;
        }
        case SupportedProtocol::UbxBinaryProtocol: {
            // send to ublx msg handler
            auto toSend = std::make_shared<ubxParcel_t>(parcel, mUbxPayloadLen.lenUint);
            ALOGV("[%s] ublx %s [size = %zu]",
                  __func__, printLog(*parcel).c_str(), parcel->size());
            pipe.Push<std::shared_ptr<ubxParcel_t>>(toSend);
            break;
        }
        default:
            // Unexpected!
            ALOGW("[%s] Some unexpected protocol", __func__);
            break;
    }
}

void TtyReader::ReadingLoop() {
    int readTry = 0;
    auto parcel = std::make_shared<std::vector<char>>();

    while (!mExitThread) {
        SupportedProtocol protocol = SupportedProtocol::UnknownProtocol;

        if (TError::Success != mTransport->Read(*parcel)) {
            ALOGI("%s, Failed to read, %d", __func__, readTry);
            std::this_thread::sleep_for(std::chrono::milliseconds(tryInterval));
            if (maxReadRetries == ++readTry) {
                ALOGE("%s, Failed to read, %d", __func__, readTry);
                if (nullptr != mDeathNotificationCallback) {
                    mDeathNotificationCallback();
                }
                return;
            }
            continue;
        }

        switch (HandleInput(parcel->back(), protocol)) {
            case RDError::Reset:
                ALOGV("%s, incomplete or lost - %s", __func__, printLog(*parcel).c_str());
                //previous message was incomplete or lost,
                //reset current parcel and continue as from begin
                ResetReader();
                parcel->clear();
                break;

            case RDError::Success:
                SendToMsgHandler(parcel, protocol);
                parcel = std::make_shared<std::vector<char>>();
                ResetReader();
                break;

            default:
                readTry = 0;
                break;
        }
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
            if (ubxLengthFirstByteOffset == mReaderUbxParcelOffset) {
                mUbxPayloadLen.lenBytes[mFirstByteOffset] = ch;
            } else if (ubxLengthSecondByteOffset == mReaderUbxParcelOffset) {
                mUbxPayloadLen.lenBytes[mSecondByteOffset] = ch;
            }

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
            break;
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
    switch (mReaderState) {
        case ReaderState::WAITING: {
            if (mNmeaBeginParcel == ch) {
                mReaderState = ReaderState::CAPTURING_NMEA;
            } else if (static_cast<char>(UbxSyncByte::UbxSync1) == ch) {
                mReaderState = ReaderState::WAITING_UBX_SYNC2;
            }

            break;
        }

        case ReaderState::WAITING_UBX_SYNC2: {
            if (static_cast<char>(UbxSyncByte::UbxSync2) == ch) {
                mReaderState = ReaderState::CAPTURING_UBX;
            } else {
                ALOGV("%s, Capturing UBX - RESET", __func__);
                // Missed UbxSync2 ? Reset the reader
                return RDError::Reset;
            }

            break;
        }

        case ReaderState::CAPTURING_NMEA: {
            if (mNmeaBeginParcel == ch) {
                ALOGV("%s, Capturing NMEA - RESET", __func__);
                // Missed end of message CR LF ? Reset the reader
                return RDError::Reset;
            }

            break;
        }

        default: {
            break;
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

    TError status = mTransport->GetTransportState();
    if (TError::TransportReady != status) {
        status = mTransport->Reset();
    }

    if (TError::TransportReady != status) {
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
