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

#define LOG_NDEBUG 1
#define LOG_TAG "GnssRenesasFakeLocationBuilder"
#include <log/log.h>

#include <chrono>
#include <cmath>
#include <utils/SystemClock.h>

#include <android/hardware/gnss/1.0/IGnss.h>

#include "include/FakeLocationBuilder.h"
#include "include/MessageQueue.h"

#define EARTH_RADIUS            6373000 // in meters
#define PI                      3.141592653589793

using std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

namespace android::hardware::gnss::V2_0::renesas {

static constexpr uint32_t numPointsNeeded = 2;
static constexpr size_t msgQueTimeout = 2000;

typedef ::android::hardware::gnss::V1_0::GnssLocation GnssLocation;
typedef ::android::hardware::gnss::V1_0::GnssLocationFlags GnssLocationFlags;

FakeLocationBuilder::FakeLocationBuilder():
    mMsgQueue(MessageQueue::GetInstance()),
    mMsgQueueCV(mMsgQueue.GetConditionVariable<FLBType>()) {}

FLBError FakeLocationBuilder::Build(LocationData& outData) {
    // Get numPointsNeeded (2) fakeLocationPoint_t from message queue
    // to set location data parameters accordingly
    std::unique_lock<std::mutex> lock(mLock);
    mMsgQueueCV.wait_for(lock, milliseconds{msgQueTimeout},
                [&] {return mMsgQueue.GetSize<FLBType>() > numPointsNeeded;});
    FLBType pt_from = mMsgQueue.Pop<FLBType>(),
            pt_to = mMsgQueue.Pop<FLBType>();

    if ((pt_from == nullptr) || (pt_to == nullptr)) {
        return FLBError::INCOMPLETE;
    }

    // Calculating bearing between points
    const double dlon = (pt_to->longitude - pt_from->longitude) * PI / 180;
    const double X = cos(pt_to->latitude * PI / 180) * sin(dlon);
    const double Y = cos(pt_from->latitude * PI / 180) *
        sin(pt_to->latitude * PI / 180) - sin(pt_from->latitude * PI / 180) *
        cos(pt_to->latitude * PI / 180) * cos(dlon);
    const float bearing = static_cast<float>(atan2(X, Y) * 180 / PI);
    outData.v1_0.horizontalAccuracyMeters = 1.f;
    outData.v1_0.gnssLocationFlags = static_cast<uint16_t>(
                GnssLocationFlags::HAS_SPEED |
                GnssLocationFlags::HAS_HORIZONTAL_ACCURACY |
                GnssLocationFlags::HAS_ALTITUDE |
                GnssLocationFlags::HAS_LAT_LONG |
                GnssLocationFlags::HAS_BEARING);
    /* ---------------------------------------------------------- */
    outData.v1_0.speedMetersPerSec = pt_from->speed;
    outData.v1_0.latitudeDegrees = pt_from->latitude;
    outData.v1_0.longitudeDegrees = pt_from->longitude;
    outData.v1_0.bearingDegrees = bearing;
    auto current_time_ms = duration_cast<milliseconds>
                    (std::chrono::system_clock::now().time_since_epoch());
    outData.v1_0.timestamp = current_time_ms.count();
    outData.elapsedRealtime.flags = ElapsedRealtimeFlags::HAS_TIMESTAMP_NS |
                                ElapsedRealtimeFlags::HAS_TIME_UNCERTAINTY_NS;
    outData.elapsedRealtime.timestampNs =
        static_cast<uint64_t>(::android::elapsedRealtimeNano());
    outData.elapsedRealtime.timeUncertaintyNs = 0;
    return FLBError::SUCCESS;
}

} // namespace android::hardware::gnss::V2_0::renesas
