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

#define LOG_TAG "GnssSalvatorHAL_AGnssRilInterface"

#include "AGnssRil.h"

namespace android {
namespace hardware {
namespace gnss {
namespace V1_0 {
namespace salvator {

AGnssRil::AGnssRil() {}

AGnssRil::~AGnssRil() {}

// Methods from ::android::hardware::gnss::V1_0::IAGnssRil follow.
Return<void> AGnssRil::setCallback(const sp<IAGnssRilCallback>&)  {
    ALOGE("%s: Not implemented", __func__);
    return Void();
}

Return<void> AGnssRil::setRefLocation(const IAGnssRil::AGnssRefLocation&)  {
    ALOGE("%s: Not implemented", __func__);
    return Void();
}

Return<bool> AGnssRil::setSetId(IAGnssRil::SetIDType, const hidl_string&)  {
    ALOGE("%s: Not implemented", __func__);
    return false;
}

Return<bool> AGnssRil::updateNetworkState(bool,
                                          IAGnssRil::NetworkType,
                                          bool) {
    ALOGE("%s: Not implemented", __func__);
    return false;
}

Return<bool> AGnssRil::updateNetworkAvailability(bool, const hidl_string&)  {
    ALOGE("%s: Not implemented", __func__);
    return false;
}

}  // namespace salvator
}  // namespace V1_0
}  // namespace gnss
}  // namespace hardware
}  // namespace android
