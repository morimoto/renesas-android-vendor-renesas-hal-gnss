/*
 * Copyright (C) 2016 The Android Open Source Project
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

#define LOG_TAG "GnssKingfisherHAL_GnssGeofencing"

#include "GnssGeofencing.h"

namespace android {
namespace hardware {
namespace gnss {
namespace V1_0 {
namespace kingfisher {

GnssGeofencing::GnssGeofencing() {}

GnssGeofencing::~GnssGeofencing() {}

// Methods from ::android::hardware::gnss::V1_0::IGnssGeofencing follow.
Return<void> GnssGeofencing::setCallback(const sp<IGnssGeofenceCallback>&)  {
    ALOGE("%s: Not implemented", __func__);
    return Void();
}

Return<void> GnssGeofencing::addGeofence(int32_t,
                             double,
                             double,
                             double,
                             IGnssGeofenceCallback::GeofenceTransition,
                             int32_t,
                             uint32_t,
                             uint32_t) {
    ALOGE("%s: Not implemented", __func__);
    return Void();
}

Return<void> GnssGeofencing::pauseGeofence(int32_t)  {
    ALOGE("%s: Not implemented", __func__);
    return Void();
}

Return<void> GnssGeofencing::resumeGeofence(int32_t, int32_t)  {
    ALOGE("%s: Not implemented", __func__);
    return Void();
}

Return<void> GnssGeofencing::removeGeofence(int32_t)  {
    ALOGE("%s: Not implemented", __func__);
    return Void();
}

}  // namespace kingfisher
}  // namespace V1_0
}  // namespace gnss
}  // namespace hardware
}  // namespace android
