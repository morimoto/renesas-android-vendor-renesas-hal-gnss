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

#ifndef GNSSINFOBUILDER_H
#define GNSSINFOBUILDER_H

#include <thread>

#include <INmeaParser.h>
#include <MessageQueue.h>

namespace android::hardware::gnss::V2_1::renesas {

/**
 * @brief IBError
 */
enum class IBError : int8_t {
    /**
     * @brief SUCCESS
     */
    SUCCESS = 0,

    /**
     * @brief INCOMPLETE - some messages are missing
     */
    INCOMPLETE = -100,

    /**
     * @brief INVALID - some messages are invalid
     */
    INVALID = -101,

    /**
     * @brief UNEXPECTED - unexpected message (out of range)
     */
    UNEXPECTED = -102,
};

/**
 * @brief GnssInfoBuilder
 */
class GnssInfoBuilder {
public:
    /**
     * @brief Construct a new Gnss Info Builder object
     */
    GnssInfoBuilder();

    /**
     * @brief Destroy the Gnss Info Builder object
     */
    ~GnssInfoBuilder();

    /**
     * @brief Build gnss info data
     *
     * @param outData - gnss info data
     * @return IBError
     */
    IBError Build(SvInfoList& outData);
protected:
    /**
     * @brief Get the Satellite object
     *
     * @param out
     * @param expectedMsgNum
     * @return IBError
     */
    IBError GetSatellite(SvInfoOutType out, uint8_t expectedMsgNum);

    /**
     * @brief Get the Fix Satellites object
     *
     */
    void GetFixSatellites();

    /**
     * @brief Get the Satellites object
     *
     */
    void GetSatellites();
private:
    GnssInfoBuilder(GnssInfoBuilder&) = delete;
    GnssInfoBuilder& operator=(const GnssInfoBuilder&) = delete;
    void ProcessFixFlag(
        android::hardware::gnss::V2_1::IGnssCallback::GnssSvInfo& sv,
        uint8_t gnssId);

    SvInfoList mSatellites[static_cast<int>(NmeaConstellationId::COUNT)];
    std::vector<int64_t> mSatellitesUsedInFix[static_cast<int>(NmeaConstellationId::COUNT)];

    MessageQueue& mMsgQueue;
    std::atomic<bool> mThreadExit;
    std::thread mGpsFixThread;
    std::thread mGpsGetThread;
    std::condition_variable& mSvInfoCV;
    std::condition_variable& mSvInfoFixCV;
    std::mutex mLock;
    std::mutex getLock;
    std::mutex fixLock;
};

}  // namespace android::hardware::gnss::V2_1::renesas

#endif // GNSSINFOBUILDER_H
