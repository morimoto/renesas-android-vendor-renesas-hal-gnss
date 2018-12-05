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

#define LOG_TAG "GnssRenesasHAL_GnssNiInterface"

#include "GnssNi.h"

namespace android {
namespace hardware {
namespace gnss {
namespace V1_0 {
namespace renesas {

GnssNi::GnssNi()   {}

GnssNi::~GnssNi() {}

// Methods from ::android::hardware::gnss::V1_0::IGnssNi follow.
Return<void> GnssNi::setCallback(const sp<IGnssNiCallback>&)  {
    ALOGE("%s: Not implemented", __func__);
    return Void();
}

Return<void> GnssNi::respond(int32_t, IGnssNiCallback::GnssUserResponseType)  {
    ALOGE("%s: Not implemented", __func__);
    return Void();
}

}  // namespace renesas
}  // namespace V1_0
}  // namespace gnss
}  // namespace hardware
}  // namespace android
