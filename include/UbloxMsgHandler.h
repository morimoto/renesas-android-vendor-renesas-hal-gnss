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

#include <IUbxParser.h>
#include <MessageQueue.h>

/**
 * @brief Ubx Sync Byte
 *
 */
enum UbxSyncByte : uint8_t {
    /**
     * @brief UbxSync1
     */
    UbxSync1 = 0xb5,

    /**
     * @brief UbxSync2
     *
     */
    UbxSync2 = 0x62,
};

/**
 * @brief ubx Length First Byte Offset
 */
const size_t ubxLengthFirstByteOffset = 4u;

/**
 * @brief ubx Length Second Byte Offset
 */
const size_t ubxLengthSecondByteOffset = 5u;

/**
 * @brief ubx Header And Checksum Len
 */
const size_t ubxHeaderAndChecksumLen = 8u;

/**
 * @brief UMHError
 */
enum class UMHError : uint8_t {
    /**
     * @brief Success
     */
    Success,

    /**
     * @brief Internal Error
     */
    InternalError,

    /**
     * @brief Failed To Process
     */
    FailedToProcess,
};

/**
 * @brief UbxParcel
 */
typedef struct UbxParcel {
    /**
     * @brief Construct a new Ubx Parcel object
     *
     * @param inSp
     * @param inLen
     */
    UbxParcel(const std::shared_ptr<std::vector<char>>& inSp,
              const uint16_t& inLen) :
        sp(inSp),
        mPayloadLen(inLen) {}
    /**
     * @brief sp
     */
    std::shared_ptr<std::vector<char>> sp;

    /**
     * @brief Payload Length
     *
     */
    uint16_t mPayloadLen;
} ubxParcel_t;

/**
 * @brief Ubx Msg Handler implementation
 *
 */
class UbxMsgHandler {
public:
    /**
     * @brief Construct a new Ubx Msg Handler object
     *
     */
    UbxMsgHandler();

    /**
     * @brief Construct a new Ubx Msg Handler object
     *
     * @param protocol
     */
    UbxMsgHandler(const UbxProtocolVersion& protocol);

    /**
     * @brief Destroy the Ubx Msg Handler object
     *
     */
    ~UbxMsgHandler();

    /**
     * @brief Update Protocol Verion
     *
     * @param protocol
     * @return UMHError
     */
    UMHError UpdateProtocolVerion(const UbxProtocolVersion& protocol);

    /**
     * @brief Start Processing
     *
     * @return UMHError
     */
    UMHError StartProcessing();

    /**
     * @brief Stop Processing
     *
     * @return UMHError
     */
    UMHError StopProcessing();

protected:
    /**
     * @brief convert Ubx To String
     *
     * @return std::string
     */
    std::string UbxToString();

    /**
     * @brief ACK Msg Parser
     *
     * @return UMHError
     */
    UMHError ACKMsgParser();

    /**
     * @brief MON Msg Parser
     *
     * @return UMHError
     */
    UMHError MONMsgParser();

    /**
     * @brief NAV Msg Parser
     *
     * @return UMHError
     */
    UMHError NAVMsgParser();

    /**
     * @brief RXM Msg Parser
     *
     * @return UMHError
     */
    UMHError RXMMsgParser();

    /**
     * @brief Select Parser
     *
     * @return UMHError
     */
    UMHError SelectParser();

    /**
     * @brief Verify CheckSum
     *
     * @param parcel
     * @param len
     * @return UMHError
     */
    UMHError VerifyCheckSum(const std::vector<char>& parcel,
                            const uint16_t& len);

    /**
     * @brief Processing Loop
     */
    void ProcessingLoop();

    /**
     * @brief Process
     *
     * @param ubxParcel
     * @return UMHError
     */
    UMHError Process(const ubxParcel_t& ubxParcel);

    /**
     * @brief Reset parser
     *
     */
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
