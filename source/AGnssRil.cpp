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

#include <AGnssRil.h>

namespace android::hardware::gnss::V2_1::renesas {
// Methods from ::android::hardware::gnss::V1_0::IAGnssRil follow.
Return<void> AGnssRil::setCallback(const
            sp<::android::hardware::gnss::V1_0::IAGnssRilCallback>& callback) {
    // TODO implement
    return Void();
}

Return<void> AGnssRil::setRefLocation(
    const ::android::hardware::gnss::V1_0::IAGnssRil::AGnssRefLocation&
    agnssReflocation) {
    // TODO implement
    return Void();
}

Return<bool> AGnssRil::setSetId(
    ::android::hardware::gnss::V1_0::IAGnssRil::SetIDType type,
    const hidl_string& setid) {
    // TODO implement
    return bool {};
}

Return<bool> AGnssRil::updateNetworkState(bool connected,
::android::hardware::gnss::V1_0::IAGnssRil::NetworkType type, bool roaming) {
    // TODO implement
    return bool {};
}

Return<bool> AGnssRil::updateNetworkAvailability(bool available,
        const hidl_string& apn) {
    // TODO implement
    return bool {};
}


// Methods from ::android::hardware::gnss::V2_0::IAGnssRil follow.
Return<bool> AGnssRil::updateNetworkState_2_0(
    const ::android::hardware::gnss::V2_0::IAGnssRil::NetworkAttributes&
    attributes) {
    // TODO implement
    return bool {};
}

}  // namespace android::hardware::gnss::V2_1::renesas
