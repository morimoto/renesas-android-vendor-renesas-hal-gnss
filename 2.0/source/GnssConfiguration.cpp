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
#define LOG_TAG "GnssRenesasConfiguration"
#include <log/log.h>

#include "include/GnssConfiguration.h"

namespace android::hardware::gnss::V2_0::renesas {
// Methods from ::android::hardware::gnss::V1_0::IGnssConfiguration follow.
Return<bool> GnssConfiguration::setSuplEs(bool enabled) {
    ALOGV("%s: GnssConfiguration is not supported", __func__);
    return false;
}

Return<bool> GnssConfiguration::setSuplVersion(uint32_t version) {
    ALOGV("%s: GnssConfiguration is not supported", __func__);
    return false;
}

Return<bool> GnssConfiguration::setSuplMode(hidl_bitfield<SuplMode> mode) {
    ALOGV("%s: GnssConfiguration is not supported", __func__);
    return false;
}

Return<bool> GnssConfiguration::setGpsLock(hidl_bitfield<GpsLock> lock) {
    ALOGV("%s: GnssConfiguration is not supported", __func__);
    return false;
}

Return<bool> GnssConfiguration::setLppProfile(hidl_bitfield<LppProfile>
        lppProfile) {
    ALOGV("%s: GnssConfiguration is not supported", __func__);
    return false;
}

Return<bool> GnssConfiguration::setGlonassPositioningProtocol(
    hidl_bitfield<GlonassPosProtocol> protocol) {
    ALOGV("%s: GnssConfiguration is not supported", __func__);
    return false;
}

Return<bool> GnssConfiguration::setEmergencySuplPdn(bool enable) {
    ALOGV("%s: GnssConfiguration is not supported", __func__);
    return false;
}


// Methods from ::android::hardware::gnss::V1_1::IGnssConfiguration follow.
Return<bool> GnssConfiguration::setBlacklist(const hidl_vec
    <::android::hardware::gnss::V1_1::IGnssConfiguration::BlacklistedSource>&
    blacklist) {
    ALOGV("%s: GnssConfiguration is not supported", __func__);
    return false;
}


// Methods from ::android::hardware::gnss::V2_0::IGnssConfiguration follow.
Return<bool> GnssConfiguration::setEsExtensionSec(uint32_t
        emergencyExtensionSeconds) {
    ALOGV("%s: GnssConfiguration is not supported", __func__);
    return false;
}

}  // namespace android::hardware::gnss::V2_0::renesas
