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
#define LOG_TAG "GnssRenesasBatching"

#include "include/GnssBatching.h"

#include <log/log.h>

namespace android::hardware::gnss::V2_1::renesas {
// Methods from ::android::hardware::gnss::V1_0::IGnssBatching follow.
Return<bool> GnssBatching::init(const
        sp<::android::hardware::gnss::V1_0::IGnssBatchingCallback>& callback) {
    ALOGV("%s: GnssBatching not implemented", __func__);
    return false;
}

Return<uint16_t> GnssBatching::getBatchSize() {
    ALOGV("%s: GnssBatching not implemented", __func__);
    return uint16_t {};
}

Return<bool> GnssBatching::start(
    const ::android::hardware::gnss::V1_0::IGnssBatching::Options& options) {
    ALOGV("%s: GnssBatching not implemented", __func__);
    return false;
}

Return<void> GnssBatching::flush() {
    ALOGV("%s: GnssBatching not implemented", __func__);
    return Void();
}

Return<bool> GnssBatching::stop() {
    ALOGV("%s: GnssBatching not implemented", __func__);
    return false;
}

Return<void> GnssBatching::cleanup() {
    ALOGV("%s: GnssBatching not implemented", __func__);
    return Void();
}


// Methods from ::android::hardware::gnss::V2_0::IGnssBatching follow.
Return<bool> GnssBatching::init_2_0(const
        sp<::android::hardware::gnss::V2_0::IGnssBatchingCallback>& callback) {
    ALOGV("%s: GnssBatching not implemented", __func__);
    return false;
}

}  // namespace android::hardware::gnss::V2_1::renesas
