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

#include "include/INmeaParser.h"
#include "include/MessageQueue.h"

enum class NMHError : uint8_t {
    Success,
    InternalError,
    FailedToProcess,
    NoCrc,
    BadCrc,
};

typedef struct NmeaParcel {
    NmeaParcel(const std::shared_ptr<std::vector<char>>& inSp) :
        sp(inSp) {}
    std::shared_ptr<std::vector<char>> sp;
} nmeaParcel_t;

class NmeaMsgHandler {
public:
    NmeaMsgHandler(const NmeaVersion& protocol);
    NmeaMsgHandler();
    ~NmeaMsgHandler();
    NMHError UpdateProtocolVersion(const NmeaVersion& protocol);
    NMHError StartProcessing();
    NMHError StopProcessing();

protected:
    NMHError SelectParser(std::string& in);
    NMHError VerifyCheckSum(nmeaParcel_t& parcel);
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
