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

#ifndef LOCATIONBUILDER_H
#define LOCATIONBUILDER_H

#include <thread>

#include "include/INmeaParser.h"
#include "include/MessageQueue.h"

namespace android::hardware::gnss::V2_1::renesas {

typedef std::map<NmeaMsgType, LocationQueueType> LBParserMap;

enum class LBError : int8_t {
    SUCCESS = 0,
    INCOMPLETE = -100,  // some messages are missing
    INVALID = -101,     // some messages are invalid
};

class LocationBuilder {
public:
    /*!
     * \brief   Constructor
     */
    LocationBuilder();

    /*!
     * \brief   Destructor
     */
    ~LocationBuilder();

    /*!
     * \brief   Builds message with GNSS location data
     *          of messages collected from receivers
     *
     * \param   outData  The structure to be filled with location data
     *
     * \return  SUCCESS - message with location was built successfully,
     *          INCOMPLETE - some messages from receivers are missing,
     *          INVALID - received GNSS data is invalid
     */
    LBError Build(LocationData& outData);
protected:
    void AddExtraInfo(LocationData& outData);
    void ProcessExtraInfo();
private:
    LocationBuilder(LocationBuilder&) = delete;
    LocationBuilder& operator=(const LocationBuilder&) = delete;

    LocationExtraInfo mExtraInfo = {};

    MessageQueue& mMsgQueue;
    std::condition_variable& mExtraInfoCV;
    std::condition_variable& mMsgQueueCV;
    std::mutex mExtraInfoLock;
    std::mutex mLock;
    std::thread mExtraInfoThread;
    std::atomic<bool> mThreadExit;
};

}  // namespace android::hardware::gnss::V2_1::renesas

#endif // LOCATIONBUILDER_H
