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

#ifndef NMEAMSGHANDLER_H
#define NMEAMSGHANDLER_H

#include <thread>

#include <INmeaParser.h>
#include <MessageQueue.h>

/**
 * @brief return error type
 */
enum class NMHError : uint8_t {
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

    /**
     * @brief No Crc
     */
    NoCrc,

    /**
     * @brief Bad Crc
     */
    BadCrc,
};

/**
 * @brief Nmea Parcel
 */
typedef struct NmeaParcel {
    /**
     * @brief Construct a new Nmea Parcel object
     *
     * @param inSp
     */
    NmeaParcel(const std::shared_ptr<std::vector<char>>& inSp) :
        sp(inSp) {}
    /**
     * @brief sp
     */
    std::shared_ptr<std::vector<char>> sp;
} nmeaParcel_t;

/**
 * @brief Nmea Msg Handler
 *
 */
class NmeaMsgHandler {
public:
    /**
     * @brief Construct a new Nmea Msg Handler object
     *
     * @param protocol
     */
    NmeaMsgHandler(const NmeaVersion& protocol);

    /**
     * @brief Construct a new Nmea Msg Handler object
     */
    NmeaMsgHandler();

    /**
     * @brief Destroy the Nmea Msg Handler object
     */
    ~NmeaMsgHandler();

    /**
     * @brief Update Protocol Version
     *
     * @param protocol
     * @return NMHError
     */
    NMHError UpdateProtocolVersion(const NmeaVersion& protocol);

    /**
     * @brief Start Processing
     *
     * @return NMHError
     */
    NMHError StartProcessing();

    /**
     * @brief Stop Processing
     *
     * @return NMHError
     */
    NMHError StopProcessing();

protected:
    /**
     * @brief Select Parser
     *
     * @param in
     * @return NMHError
     */
    NMHError SelectParser(std::string& in);

    /**
     * @brief Verify CheckSum
     *
     * @param parcel
     * @return NMHError
     */
    NMHError VerifyCheckSum(nmeaParcel_t& parcel);

    /**
     * @brief Processing Loop
     *
     */
    void ProcessingLoop();

private:
    std::atomic<bool> mExitThread;
    std::thread mProcessingThread;
    std::mutex mLock;
    NmeaVersion mProtocol = NMEAv23;
    MessageQueue& mPipe;
    std::condition_variable& mWaitCv;
    const char crcDelimiter = '*';
};

#endif // NMEAMSGHANDLER_H
