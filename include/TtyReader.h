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
#pragma once

#include <thread>
#include <atomic>

#include "include/IReader.h"
#include "include/IGnssReceiver.h"

using namespace android::hardware::gnss::V2_0::renesas;

class TtyReader : public IReader {
public:
    TtyReader(std::shared_ptr<Transport> transport);
    ~TtyReader() override;

    RDError Start() override;
    RDError Stop() override;
    RDError SetUpNotificationCallback(cbPtr notificationCb) override;
protected:
    TtyReader() = delete;
    void ReadingLoop();
    RDError HandleInput(const char& ch, SupportedProtocol& protocol);
    void ResetReader();
    RDError CaptureParcel(const char& ch, SupportedProtocol& protocol);
private:
    enum class ReaderState : uint8_t {
        WAITING,
        CAPTURING_NMEA,
        WAITING_UBX_SYNC2,
        CAPTURING_UBX
    };

    union UbxPayloadLen {
        char lenBytes[sizeof(uint16_t)];
        uint16_t lenUint;
    };

    const uint8_t mLittleEndianFirsByteOffset = 0u;
    const uint8_t mLittleEndianSecondByteOffset = 1u;
    const uint8_t mBigEndianFistByteOffset = 1u;
    const uint8_t mBigEndianSecondByteOffset = 0u;

    cbPtr mDeathNotificationCallback;
    std::thread mReadingLoop;
    std::shared_ptr<Transport> mTransport;
    std::atomic<bool> mExitThread;

    UbxPayloadLen mUbxPayloadLen = {};
    ReaderState mReaderState = ReaderState::WAITING;
    size_t mReaderUbxParcelOffset = 0u;
    Endian mEndianType = Endian::Unset;
};
