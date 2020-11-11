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

#ifndef AGNSSRIL_H
#define AGNSSRIL_H

#include <android/hardware/gnss/2.0/IAGnssRil.h>

namespace android::hardware::gnss::V2_1::renesas {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

/**
 * Extended interface for AGNSS RIL support. An Assisted GNSS Radio Interface
 * Layer interface allows the GNSS chipset to request radio interface layer
 * information from Android platform. Examples of such information are reference
 * location, unique subscriber ID, phone number string and network availability changes.
 */
struct AGnssRil : public android::hardware::gnss::V2_0::IAGnssRil {
    Return<void> setCallback(const
        sp<::android::hardware::gnss::V1_0::IAGnssRilCallback>&
        callback) override;
    Return<void> setRefLocation(
        const ::android::hardware::gnss::V1_0::IAGnssRil::AGnssRefLocation&
        agnssReflocation) override;
    Return<bool> setSetId(::android::hardware::gnss::V1_0::IAGnssRil::SetIDType
                          type, const hidl_string& setid) override;
    Return<bool> updateNetworkState(bool connected,
            ::android::hardware::gnss::V1_0::IAGnssRil::NetworkType type,
            bool roaming) override;
    Return<bool> updateNetworkAvailability(bool available,
                                           const hidl_string& apn) override;

    // Methods from ::android::hardware::gnss::V2_0::IAGnssRil follow.
    Return<bool> updateNetworkState_2_0(const
        ::android::hardware::gnss::V2_0::IAGnssRil::NetworkAttributes&
        attributes) override;

};

}  // namespace android::hardware::gnss::V2_1::renesas

#endif // AGNSSRIL_H
