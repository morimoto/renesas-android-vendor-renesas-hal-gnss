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

#include "include/NmeaGga.h"

#include <android/hardware/gnss/1.0/types.h>
#include <android/hardware/gnss/2.0/types.h>

using GnssLocationFlags = android::hardware::gnss::V1_0::GnssLocationFlags;
using LocationData_v1 = android::hardware::gnss::V1_0::GnssLocation;
using LocationData_v2 = android::hardware::gnss::V2_0::GnssLocation;

template <>
NPError NmeaGga<LocationExtraInfoOutType>::GetData(LocationExtraInfoOutType out) {
    if (!IsValid()) {
        return NPError::InvalidData;
    }

    out.flags |= GnssLocationFlags::HAS_ALTITUDE;
    out.altitude = mParcel.altitude;
    ALOGV("%s, Success", __PRETTY_FUNCTION__);
    return NPError::Success;
}
