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

#ifndef UBLOXMSGHANDLER_H
#define UBLOXMSGHANDLER_H

#include <thread>

#include "include/IUbxParser.h"
#include "include/MessageQueue.h"

enum UbxSyncByte : uint8_t {
    UbxSync1 = 0xb5,
    UbxSync2 = 0x62,
};

const size_t ubxLengthFirstByteOffset = 4u;
const size_t ubxLengthSecondByteOffset = 5u;
const size_t ubxHeaderAndChecksumLen = 8u;

enum class UMHError : uint8_t {
    Success,
    InternalError,
    FailedToProcess,
};

typedef struct UbxParcel {
    UbxParcel(const std::shared_ptr<std::vector<char>>& inSp,
              const uint16_t& inLen) :
        sp(inSp),
        mPayloadLen(inLen) {}
    std::shared_ptr<std::vector<char>> sp;
    uint16_t mPayloadLen;
} ubxParcel_t;

//TODO(g.chabukiani): add doxygen, check all over the project
class UbxMsgHandler {
public:
    UbxMsgHandler();
    UbxMsgHandler(const UbxProtocolVersion& protocol);
    ~UbxMsgHandler();
    UMHError UpdateProtocolVerion(const UbxProtocolVersion& protocol);
    UMHError StartProcessing();
    UMHError StopProcessing();

protected:
    UMHError SelectParser();
    UMHError VerifyCheckSum(const std::vector<char>& parcel,
                            const uint16_t& len);
    void ProcessingLoop();
    UMHError Process(const ubxParcel_t& ubxParcel);
    void Reset();

private:
    enum UbxParcelOffsets : size_t {
        classOffset = 2,
        endLeftShifCrc = 2,
        idOffset = 3,
        payloadOffset = 6,
        checksum2Offset = 1,
    };

    typedef struct ProcessingMsg {
        UbxClass rxClass;
        UbxId rxId;
        uint8_t rxChecksumA; // checksum that we get in the message itself
        uint8_t rxChecksumB; // checksum that we get in the message itself
        size_t lenght;
        char* msgPayload; // TODO(g.chabukiani): should replace or rework
    } processingMsg_t;

    processingMsg_t mParcel = {};

    std::atomic<bool> mExitThread;
    std::thread mProcessingThread;
    std::mutex mLock;
    //TODO(g.chabukiani): update parsers to use ubx protocol versions
    [[maybe_unused]] UbxProtocolVersion mProtocol = UbxProtocolVersion::UBX14;
    MessageQueue& mPipe;
    std::condition_variable& mWaitCv;
};

#endif // UBLOXMSGHANDLER_H
