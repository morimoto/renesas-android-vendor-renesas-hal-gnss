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
#define LOG_NDEBUG 1
#define LOG_TAG "GnssRenesasInfoBuilder"
#include <log/log.h>

#include <chrono>
#if !LOG_NDEBUG
#include <iomanip>
#endif

#include "include/GnssInfoBuilder.h"

using std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using GnssSvInfo = ::android::hardware::gnss::V2_0::IGnssCallback::GnssSvInfo;
using ::android::hardware::gnss::V2_0::GnssConstellationType;

#if !LOG_NDEBUG
[[maybe_unused]]
void PrintSvList(const std::vector<GnssSvInfo>& svlist,
                                         const uint8_t constellation) {
    std::stringstream ss;

    for (const auto& sv : svlist) {
        ss << std::dec << sv.v1_0.svid << "[";
        ss << static_cast<uint16_t>(sv.constellation) << ", ";
        ss << static_cast<uint16_t>(sv.v1_0.constellation) << ", ";
        ss << sv.v1_0.cN0Dbhz << ", ";
        ss << sv.v1_0.elevationDegrees << ", ";
        ss << sv.v1_0.azimuthDegrees << ", ";
        ss << std::hex << std::showbase << static_cast<uint16_t>(sv.v1_0.svFlag) << "], ";
    }

    ALOGV("constellation: %hhu, buffer: %s", constellation, ss.str().c_str());
}
#endif

namespace android::hardware::gnss::V2_0::renesas {

GnssInfoBuilder::GnssInfoBuilder():
        mMsgQueue(MessageQueue::GetInstance()),
        mSvInfoCV(mMsgQueue.GetConditionVariable<SvInfoQueueType>()),
        mSvInfoFixCV(mMsgQueue.GetConditionVariable<SvInfoFixQueueType>()) {
    mThreadExit = false;
    if (!mGpsFixThread.joinable()) {
        mGpsFixThread = std::thread(&GnssInfoBuilder::GetFixSatellites, this);
    }
    if (!mGpsGetThread.joinable()) {
        mGpsGetThread = std::thread(&GnssInfoBuilder::GetSatellites, this);
    }
}

GnssInfoBuilder::~GnssInfoBuilder() {
    mSvInfoCV.notify_one();
    mSvInfoFixCV.notify_one();

    mThreadExit = true;
    if (mGpsFixThread.joinable()) {
        mGpsFixThread.join();
    }
    if (mGpsGetThread.joinable()) {
        mGpsGetThread.join();
    }
}

void GnssInfoBuilder::GetFixSatellites() {
    ALOGV("%s", __func__);

    while (!mThreadExit) {
        if (mMsgQueue.Empty<SvInfoFixQueueType>()) {
            std::unique_lock<std::mutex> lock(mLock);
            mSvInfoFixCV.wait(lock,
                    [&] {return !mMsgQueue.Empty<SvInfoFixQueueType>();});
        }

        auto parcel = mMsgQueue.Pop<SvInfoFixQueueType>();

        if (!parcel->IsValid()) {
            continue;
        }

        SvInfoFixType fixSatellitesList;
        parcel->GetData(fixSatellitesList);

        if (fixSatellitesList.gnssId >=
                    static_cast<size_t>(NmeaConstellationId::ANY)) {
            ALOGV("Unknown GSA GNSS system ID");
            continue;
        }
        std::lock_guard<std::mutex> lock(fixLock);
        mSatellitesUsedInFix[fixSatellitesList.gnssId] =
                                            fixSatellitesList.svList;
    }
}

IBError GnssInfoBuilder::GetSatellite(SvInfoOutType out,
                                      uint8_t expectedMsgNum) {
    if (mMsgQueue.Empty<SvInfoQueueType>()) {
        std::unique_lock<std::mutex> lock(mLock);
        mSvInfoCV.wait(lock,
                       [&] { return !mMsgQueue.Empty<SvInfoQueueType>(); });
    }
    auto parcel = mMsgQueue.Pop<SvInfoQueueType>();
    if (parcel == nullptr) {
        return IBError::INCOMPLETE;
    } else if (!parcel->IsValid()) {
        return IBError::INVALID;
    }

    parcel->GetData(out);
    if (out.msgNum != expectedMsgNum) {
        return IBError::UNEXPECTED;
    }

    return IBError::SUCCESS;
}

void GnssInfoBuilder::GetSatellites() {
    ALOGV("%s", __func__);
    while (!mThreadExit) {
        SvInfoType sv;
        uint8_t currMsgNum = 1;
        uint8_t gnssId;
        bool complete = false;

        while (IBError::SUCCESS == GetSatellite(sv, currMsgNum++)) {
            if (sv.msgNum == 1) {
                gnssId = sv.gnssId;
            } else if (gnssId != sv.gnssId) {
                break;
            }

            if (sv.msgAmount == sv.msgNum) {
                complete = true;
                break;
            }
        }
        if (!complete) {
            continue;
        }
        ALOGV("%lu satellites with constellation id %zd", sv.svInfoList.size(), sv.gnssId);
        std::lock_guard<std::mutex> lock(getLock);
        mSatellites[gnssId] = sv.svInfoList;
    }
}

void GnssInfoBuilder::ProcessFixFlag(GnssSvInfo& sv, uint8_t gnssId) {
    uint32_t origSvid = sv.v1_0.svid;
    if (NmeaConstellationId::GLONASS == gnssId){
        origSvid += 64;
    } else if (origSvid >= 140 && origSvid <= 171) {
        origSvid -= 87;
    } else if (origSvid >= 183 && origSvid <= 189) {
        origSvid -= 31;
    }
    std::lock_guard<std::mutex> lock(fixLock);
    std::vector<int64_t>* usedInFix = &mSatellitesUsedInFix[gnssId];
    for (auto it = usedInFix->begin(); it != usedInFix->end();) {
        if (*it == origSvid) {
            sv.v1_0.svFlag |= static_cast<uint8_t>
                            (IGnssCallback::GnssSvFlags::USED_IN_FIX);
            it = usedInFix->erase(it);
            return ;
        } else {
            it++;
        }
    }
}

IBError GnssInfoBuilder::Build(SvInfoList& outList) {
    ALOGV("%s", __func__);

    unsigned int svCount = 0;
    std::lock_guard<std::mutex> lock(getLock);
    for (uint8_t satType = 0; satType < static_cast<int>(NmeaConstellationId::COUNT); satType++) {
        for (size_t satNum = 0; satNum < mSatellites[satType].size(); satNum++) {
            ProcessFixFlag(mSatellites[satType][satNum], satType);
            outList.push_back(mSatellites[satType][satNum]);
            if (++svCount >= static_cast<unsigned int>(V1_0::GnssMax::SVS_COUNT)) {
                satType = static_cast<int>(NmeaConstellationId::COUNT);
                break;
            }
        }
    }

    ALOGV("GPS SV: GPS/SBAS/QZSS:%lu GLONASS:%lu GALILEO:%lu BEIDOU:%lu UNKNOWN:%lu total:%u",
        mSatellites[0].size(), mSatellites[1].size(), mSatellites[2].size(), mSatellites[3].size(),
        mSatellites[4].size(), svCount);

    return IBError::SUCCESS;
}

}  // namespace android::hardware::gnss::V2_0::renesas
