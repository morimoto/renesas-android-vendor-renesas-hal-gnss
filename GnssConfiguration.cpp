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

#define LOG_TAG "GnssKingfisherHAL_GnssConfigurationInterface"

#include <log/log.h>

#include "GnssConfiguration.h"

namespace android {
namespace hardware {
namespace gnss {
namespace V1_0 {
namespace kingfisher {

GnssConfiguration::GnssConfiguration() {}

// Methods from ::android::hardware::gps::V1_0::IGnssConfiguration follow.
Return<bool> GnssConfiguration::setSuplEs(bool)  {
    ALOGE("%s: Not implemented", __func__);
    return false;
}

Return<bool> GnssConfiguration::setSuplVersion(uint32_t)  {
    ALOGE("%s: Not implemented", __func__);
    return false;
}

Return<bool> GnssConfiguration::setSuplMode(uint8_t)  {
    ALOGE("%s: Not implemented", __func__);
    return false;
}

Return<bool> GnssConfiguration::setLppProfile(uint8_t) {
    ALOGE("%s: Not implemented", __func__);
    return false;
}

Return<bool> GnssConfiguration::setGlonassPositioningProtocol(uint8_t) {
    ALOGE("%s: Not implemented", __func__);
    return false;
}

Return<bool> GnssConfiguration::setGpsLock(uint8_t) {
    ALOGE("%s: Not implemented", __func__);
    return false;
}

Return<bool> GnssConfiguration::setEmergencySuplPdn(bool) {
    ALOGE("%s: Not implemented", __func__);
    return false;
}

}  // namespace kingfisher
}  // namespace V1_0
}  // namespace gnss
}  // namespace hardware
}  // namespace android
