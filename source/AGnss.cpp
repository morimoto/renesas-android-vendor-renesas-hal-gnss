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

#include <AGnss.h>

namespace android::hardware::gnss::V2_1::renesas {
// Methods from ::android::hardware::gnss::V2_0::IAGnss follow.
Return<void> AGnss::setCallback(const
            sp<::android::hardware::gnss::V2_0::IAGnssCallback>& callback) {
    // TODO implement
    return Void();
}

Return<bool> AGnss::dataConnClosed() {
    // TODO implement
    return bool {};
}

Return<bool> AGnss::dataConnFailed() {
    // TODO implement
    return bool {};
}

Return<bool> AGnss::setServer(
    ::android::hardware::gnss::V2_0::IAGnssCallback::AGnssType type,
    const hidl_string& hostname, int32_t port) {
    // TODO implement
    return bool {};
}

Return<bool> AGnss::dataConnOpen(uint64_t networkHandle,
                const hidl_string& apn,
                ::android::hardware::gnss::V2_0::IAGnss::ApnIpType apnIpType) {
    // TODO implement
    return bool {};
}

}  // namespace android::hardware::gnss::V2_1::renesas
