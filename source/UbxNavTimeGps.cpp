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

#include "include/UbxNavTimeGps.h"

using GnssData =
    android::hardware::gnss::V2_0::IGnssMeasurementCallback::GnssData;

// Difference between hardware clock epoch time and GPS epoch time in hours
const long kGpsTimeBaseDifferenceHours = 87768;
const long long kSecondsInWeek = 7 * 24 * 60 * 60;
const int64_t kNanoSecondsInOneSecond = 1000000000;

using std::chrono::hours;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;
using std::chrono::time_point;

static time_t GpsTimeInSeconds(const int16_t week, const uint32_t iTow) {
    time_point<system_clock> time(week * seconds(kSecondsInWeek) +
                                  milliseconds(iTow));
    return system_clock::to_time_t(time);
}

static time_t GpsTimeToSystemTime(const int16_t week, const uint32_t iTow) {
    time_point<system_clock> time(hours(kGpsTimeBaseDifferenceHours) +
                                  week * seconds(kSecondsInWeek) +
                                  milliseconds(iTow));
    return system_clock::to_time_t(time);
}

template <>
UPError UbxNavTimeGps<GnssData*>::GetData(GnssData* out) {
    if (!mIsValid) {
        return UPError::InvalidData;
    }

    out->clock.timeNs = GpsTimeToSystemTime(mParcel.week, mParcel.iTow) *
                            kNanoSecondsInOneSecond +
                        mParcel.fTow;
    // Full bias will be difference between system and GPS time
    // as receiver local time is a mapping of
    // the local 1 kHz reference onto a GNSS time-base - 9.1 Receiver Local Time
    out->clock.fullBiasNs =
        out->clock.timeNs -
        GpsTimeInSeconds(mParcel.week, mParcel.iTow) * kNanoSecondsInOneSecond +
        mParcel.fTow;
    out->clock.biasNs            = mParcel.fTow;
    out->clock.biasUncertaintyNs = static_cast<double>(mParcel.tAcc);
    out->clock.timeUncertaintyNs = static_cast<double>(mParcel.tAcc);
    out->clock.leapSecond        = mParcel.leapS;
    out->clock.gnssClockFlags    = mClockFlags;
    return UPError::Success;
}
