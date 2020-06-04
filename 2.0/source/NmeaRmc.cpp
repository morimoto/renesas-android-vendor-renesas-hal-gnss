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

#include "include/NmeaRmc.h"
#include <android/hardware/gnss/1.0/types.h>
#include <android/hardware/gnss/2.0/types.h>

using LocationData_v1 = android::hardware::gnss::V1_0::GnssLocation;
using LocationData_v2 = android::hardware::gnss::V2_0::GnssLocation;

template <>
NPError NmeaRmc<LocationOutType>::GetData(LocationOutType out) {
    if (!IsValid()) {
        return NPError::InvalidData;
    }

    out.v1_0.gnssLocationFlags = mFlags;
    out.v1_0.timestamp = mParcel.time;
    out.v1_0.latitudeDegrees = mParcel.lat;
    out.v1_0.longitudeDegrees = mParcel.lon;
    out.v1_0.speedMetersPerSec = mParcel.speed;
    out.v1_0.bearingDegrees = mParcel.cog;
    out.v1_0.speedAccuracyMetersPerSecond = mSpeedAccuracy;
    out.v1_0.bearingAccuracyDegrees = mBearingAccuracy;
    ALOGV("%s, Success", __PRETTY_FUNCTION__);
    return NPError::Success;
}

template <>
NPError NmeaRmc<LocationData_v1*>::GetData(LocationData_v1* out) {
    if (!IsValid()) {
        return NPError::InvalidData;
    }

    out->gnssLocationFlags = mFlags & std::numeric_limits<uint16_t>::max();
    out->timestamp = mParcel.time;
    out->latitudeDegrees = mParcel.lat;
    out->longitudeDegrees = mParcel.lon;
    out->speedMetersPerSec = mParcel.speed;
    out->bearingDegrees = mParcel.cog;
    out->speedAccuracyMetersPerSecond = mSpeedAccuracy;
    out->bearingAccuracyDegrees = mBearingAccuracy;
    ALOGV("%s, Success", __PRETTY_FUNCTION__);
    return NPError::Success;
}
