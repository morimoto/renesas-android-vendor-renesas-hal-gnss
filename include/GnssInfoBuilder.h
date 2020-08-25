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

#include "include/INmeaParser.h"
#include "include/MessageQueue.h"

namespace android::hardware::gnss::V2_0::renesas {

enum class IBError : int8_t {
    SUCCESS = 0,
    INCOMPLETE = -100,  // some messages are missing
    INVALID = -101,     // some messages are invalid
    UNEXPECTED = -102,  // unexpected message (out of range)
};


class GnssInfoBuilder {
public:
    GnssInfoBuilder();
    ~GnssInfoBuilder();
    IBError Build(SvInfoList& outData);
protected:
    IBError GetSatellite(SvInfoOutType out, uint8_t expectedMsgNum);
    void GetFixSatellites();
    void GetSatellites();
private:
    GnssInfoBuilder(GnssInfoBuilder&) = delete;
    GnssInfoBuilder& operator=(const GnssInfoBuilder&) = delete;
    void ProcessFixFlag(IGnssCallback::GnssSvInfo& sv, uint8_t gnssId);

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

}  // namespace android::hardware::gnss::V2_0::renesas
