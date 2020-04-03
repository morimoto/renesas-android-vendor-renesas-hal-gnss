/*
 * Copyright (C) 2019 GlobalLogic
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

#include <memory>
#include <map>
#include <cstdint>

#include <android/hardware/gnss/2.0/types.h>
#include "include/IReader.h"
#include "include/MessageQueue.h"

namespace android::hardware::gnss::V2_0::renesas {

using LocationData = ::android::hardware::gnss::V2_0::GnssLocation;
typedef std::shared_ptr<fakeLocationPoint_t> FLBType;

enum class FLBError : int8_t {
    SUCCESS = 0,
    INCOMPLETE = -100,  // some messages are missing
    INVALID = -101,     // some messages are invalid
};

class FakeLocationBuilder {
public:
    /*!
     * \brief   Constructor
     */
    FakeLocationBuilder();

    /*!
     * \brief   Destructoor
     */
    ~FakeLocationBuilder() = default;

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
    FLBError Build(LocationData& outData);
private:
    FakeLocationBuilder(FakeLocationBuilder&) = delete;
    FakeLocationBuilder& operator=(const FakeLocationBuilder&) = delete;
    MessageQueue& mMsgQueue;
    std::condition_variable& mMsgQueueCV;
    std::mutex mLock;
};

}  // namespace android::hardware::gnss::V2_0::renesas