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

#include <NmeaGga.h>

#include <log/log.h>

using GnssLocationFlags = android::hardware::gnss::V1_0::GnssLocationFlags;
using LocationData_v1 = android::hardware::gnss::V1_0::GnssLocation;
using LocationData_v2 = android::hardware::gnss::V2_0::GnssLocation;

static const uint16_t flagsToClear = GnssLocationFlags::HAS_ALTITUDE |
                                     GnssLocationFlags::HAS_HORIZONTAL_ACCURACY;

template <>
NPError NmeaGga<LocationExtraInfoOutType>::GetData(LocationExtraInfoOutType out) {
    if (!IsValid()) {
        return NPError::InvalidData;
    }

    out.flags &= ~flagsToClear;
    out.flags |= mFlags;
    out.altitude = mParcel.altitude;

    /*
    * The NMEA protocol does not report accuracy.
    * For this reason, we take some stated accuracy of the device and
    * multiply it by Horizontal Dilution of Precision.
    * This value will be overwritten with real accuracy
    * from the PUBX message if it is supported by the receiver.
    */
    out.horizontalAcc = mParcel.hdop * 2.5f;
    ALOGV("%s, Success", __PRETTY_FUNCTION__);
    return NPError::Success;
}
