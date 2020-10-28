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
#define LOG_TAG "GnssRenesasLocationBuilder"

#include <LocationBuilder.h>

#include <log/log.h>
#include <utils/SystemClock.h>

namespace android::hardware::gnss::V2_1::renesas {

static constexpr size_t msgQueTimeout = 1000;

LocationBuilder::LocationBuilder():
    mMsgQueue(MessageQueue::GetInstance()),
    mExtraInfoCV(mMsgQueue.GetConditionVariable<LocationExtraInfoQueueType>()),
    mMsgQueueCV(mMsgQueue.GetConditionVariable<LocationQueueType>()) {
    if (!mExtraInfoThread.joinable()) {
        mExtraInfoThread = std::thread(&LocationBuilder::ProcessExtraInfo, this);
    }
}

LocationBuilder::~LocationBuilder() {
    mThreadExit = true;
    mMsgQueueCV.notify_one();
    mExtraInfoCV.notify_one();

    if (mExtraInfoThread.joinable()) {
        mExtraInfoThread.join();
    }
}

void LocationBuilder::ProcessExtraInfo() {
    while (!mThreadExit) {
        std::unique_lock<std::mutex> lock(mExtraInfoLock);
        mExtraInfoCV.wait(lock,
                [&] {return !mMsgQueue.Empty<LocationExtraInfoQueueType>();});

        auto parcel = mMsgQueue.Pop<LocationExtraInfoQueueType>();
        if (!parcel->IsValid()) {
            continue;
        }

        parcel->GetData(mExtraInfo);
    }
}

void LocationBuilder::AddExtraInfo(LocationData& outData) {
    std::unique_lock<std::mutex> lock(mExtraInfoLock);

    outData.v1_0.gnssLocationFlags |= mExtraInfo.flags;
    outData.v1_0.altitudeMeters = mExtraInfo.altitude;
    outData.v1_0.horizontalAccuracyMeters = mExtraInfo.horizontalAcc;
    outData.v1_0.verticalAccuracyMeters = mExtraInfo.verticalAcc;

    outData.elapsedRealtime.flags =
        android::hardware::gnss::V2_0::ElapsedRealtimeFlags::HAS_TIMESTAMP_NS |
        android::hardware::gnss::V2_0::ElapsedRealtimeFlags::HAS_TIME_UNCERTAINTY_NS;
    outData.elapsedRealtime.timestampNs =
        static_cast<uint64_t>(::android::elapsedRealtimeNano());
    outData.elapsedRealtime.timeUncertaintyNs = 0;
}

LBError LocationBuilder::Build(LocationData& outData) {
    std::unique_lock<std::mutex> lock(mLock);
    mMsgQueueCV.wait_for(lock, std::chrono::milliseconds{msgQueTimeout},
                [&] {return !mMsgQueue.Empty<LocationQueueType>();});

    auto parcel = mMsgQueue.Pop<LocationQueueType>();

    if (parcel == nullptr) {
        return LBError::INCOMPLETE;
    } else if (!parcel->IsValid()) {
        return LBError::INVALID;
    }

    parcel->GetData(outData);
    AddExtraInfo(outData);

    return LBError::SUCCESS;
}

}  // namespace android::hardware::gnss::V2_1::renesas
