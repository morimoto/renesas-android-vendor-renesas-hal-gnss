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

#ifndef TTYREADER_H
#define TTYREADER_H

#include <thread>

#include <IGnssReceiver.h>
#include <IReader.h>

using namespace android::hardware::gnss::V2_1::renesas;

/**
 * @brief TtyReader implementation
 */
class TtyReader : public IReader {
public:
    /**
     * @brief Construct a new Tty Reader object
     *
     * @param transport
     */
    TtyReader(std::shared_ptr<Transport> transport);
    ~TtyReader() override;

    RDError Start() override;
    RDError Stop() override;
    RDError SetUpNotificationCallback(cbPtr notificationCb) override;
protected:
    TtyReader() = delete;

    /**
     * @brief Reading Loop
     *
     */
    void ReadingLoop();

    /**
     * @brief Handle Input
     *
     * @param ch input char
     * @param protocol
     * @return RDError
     */
    RDError HandleInput(const char& ch, SupportedProtocol& protocol);

    /**
     * @brief Reset Reader
     *
     */
    void ResetReader();

    /**
     * @brief Capture Parcel
     *
     * @param ch
     * @param protocol
     * @return RDError
     */
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

    const std::shared_ptr<Transport> mTransport;
    const Endian  mEndianType;
    const uint8_t mFirstByteOffset;
    const uint8_t mSecondByteOffset;

    cbPtr mDeathNotificationCallback;
    std::thread mReadingLoop;
    std::atomic<bool> mExitThread;

    UbxPayloadLen mUbxPayloadLen = {};
    ReaderState mReaderState = ReaderState::WAITING;
    size_t mReaderUbxParcelOffset = 0u;
    void SendToMsgHandler(const std::shared_ptr<std::vector<char>>& parcel,
                          const SupportedProtocol& protocol);
};

#endif // TTYREADER_H
