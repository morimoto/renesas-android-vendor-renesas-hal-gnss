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

#ifndef FAKELOCATIONBUILDER_H
#define FAKELOCATIONBUILDER_H

#include <android/hardware/gnss/2.0/types.h>

#include <IReader.h>
#include <MessageQueue.h>

namespace android::hardware::gnss::V2_1::renesas {

/**
 * @brief GNSS location data
 */
using LocationData = ::android::hardware::gnss::V2_0::GnssLocation;

/**
 * @brief FLBType
 */
typedef std::shared_ptr<fakeLocationPoint_t> FLBType;

/**
 * @brief return type
 */
enum class FLBError : int8_t {
    /**
     * @brief message with location was built successfully
     */
    SUCCESS = 0,
    /**
     * @brief some messages from receivers are missing
     */
    INCOMPLETE = -100,  // some messages are missing
    /**
     * @brief received GNSS data is invalid
     */
    INVALID = -101,  // some messages are invalid
};

/**
 * @brief Fake location builder class
 *
 */
class FakeLocationBuilder {
public:
    /**
     * @brief   Constructor
     */
    FakeLocationBuilder();

    /**
     * @brief   Destructoor
     */
    ~FakeLocationBuilder() = default;

    /**
     * @brief   Builds message with GNSS location data
     *          of messages collected from receivers
     *
     * @param   outData  The structure to be filled with location data
     *
     * @return  SUCCESS - message with location was built successfully,
     *          INCOMPLETE - some messages from receivers are missing,
     *          INVALID - received GNSS data is invalid
     */
    FLBError Build(std::queue<LocationData>& outData);

    /**
     * @brief Builds message with GNSS location data
     *       of messages collected from file
     *
     * @param pt_from
     * @param pt_to
     * @param outData The structure to be filled with location data
     * @return  SUCCESS - message with location was built successfully,
     *          INCOMPLETE - some messages from receivers are missing,
     *          INVALID - received GNSS data is invalid
     */
    FLBError Build(fakeLocationPoint_t& pt_from,
            fakeLocationPoint_t& pt_to, std::queue<LocationData>& outData);
private:
    FakeLocationBuilder(FakeLocationBuilder&) = delete;
    FakeLocationBuilder& operator=(const FakeLocationBuilder&) = delete;
    MessageQueue& mMsgQueue;
    std::condition_variable& mMsgQueueCV;
    std::mutex mLock;
};

}  // namespace android::hardware::gnss::V2_1::renesas

#endif // FAKELOCATIONBUILDER_H
