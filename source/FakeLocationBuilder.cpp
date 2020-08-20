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
#define LOG_TAG "GnssRenesasFakeLocationBuilder"

#include "include/FakeLocationBuilder.h"

#include <log/log.h>
#include <utils/SystemClock.h>

#include <chrono>
#include <cmath>

#define EARTH_RADIUS            6373000 // in meters
#define PI                      3.141592653589793

using std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

namespace android::hardware::gnss::V2_1::renesas {

typedef ::android::hardware::gnss::V1_0::GnssLocation GnssLocation;
typedef ::android::hardware::gnss::V1_0::GnssLocationFlags GnssLocationFlags;

FakeLocationBuilder::FakeLocationBuilder():
    mMsgQueue(MessageQueue::GetInstance()),
    mMsgQueueCV(mMsgQueue.GetConditionVariable<FLBType>()) {}

FLBError FakeLocationBuilder::Build(fakeLocationPoint_t& from,
        fakeLocationPoint_t& to, std::queue<LocationData>& outData) {
    fakeLocationPoint_t pt_from = from,
        pt_to = to;

    LocationData location;

    location.v1_0.speedMetersPerSec = pt_from.speed;
    location.v1_0.latitudeDegrees = pt_from.latitude;
    location.v1_0.longitudeDegrees = pt_from.longitude;

    const double dlon = (pt_to.longitude - pt_from.longitude) * PI / 180;
    const double dlat = (pt_to.latitude - pt_from.latitude) * PI / 180;
    const double a = pow((sin(dlat / 2)), 2) + cos(pt_from.latitude * PI / 180) *
        cos(pt_to.latitude * PI / 180) * pow((sin(dlon / 2)), 2);
    const double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    const double d = EARTH_RADIUS * c; // Distance
    const double t = d / location.v1_0.speedMetersPerSec; // Time in seconds to reach last point
    // lat lon step for next reporting point
    const double latStep = (pt_to.latitude - pt_from.latitude) / t;
    const double lonStep = (pt_to.longitude - pt_from.longitude) / t;

    // Calculating bearing between points
    const double X = cos(pt_to.latitude * PI / 180) * sin(dlon);
    const double Y = cos(pt_from.latitude * PI / 180) *
        sin(pt_to.latitude * PI / 180) - sin(pt_from.latitude * PI / 180) *
        cos(pt_to.latitude * PI / 180) * cos(dlon);
    const float bearing = static_cast<float>(atan2(X, Y) * 180 / PI);

    location.v1_0.bearingDegrees = bearing;
    for (int i = 0; i <= static_cast<int>(t); i++) {
        location.v1_0.horizontalAccuracyMeters = 1.f;
        location.v1_0.gnssLocationFlags = static_cast<uint16_t>(
                    GnssLocationFlags::HAS_SPEED |
                    GnssLocationFlags::HAS_HORIZONTAL_ACCURACY |
                    GnssLocationFlags::HAS_ALTITUDE |
                    GnssLocationFlags::HAS_LAT_LONG |
                    GnssLocationFlags::HAS_BEARING);
        /* ---------------------------------------------------------- */
        if (i == static_cast<int>(t)) {
            location.v1_0.latitudeDegrees = pt_to.latitude;
            location.v1_0.longitudeDegrees = pt_to.longitude;
            location.v1_0.speedMetersPerSec = d - (static_cast<int>(t)) * pt_from.speed;
        }
        else {
            location.v1_0.latitudeDegrees += latStep;
            location.v1_0.longitudeDegrees += lonStep;
        }
        auto current_time_ms = duration_cast<milliseconds>
                        (std::chrono::system_clock::now().time_since_epoch());
        location.v1_0.timestamp = current_time_ms.count();
        location.elapsedRealtime.flags =
            android::hardware::gnss::V2_0::ElapsedRealtimeFlags::HAS_TIMESTAMP_NS |
            android::hardware::gnss::V2_0::ElapsedRealtimeFlags::HAS_TIME_UNCERTAINTY_NS;
        location.elapsedRealtime.timestampNs =
            static_cast<uint64_t>(::android::elapsedRealtimeNano());
        location.elapsedRealtime.timeUncertaintyNs = 0;
        outData.push(location);
    }
    return FLBError::SUCCESS;
}

} // namespace android::hardware::gnss::V2_1::renesas
