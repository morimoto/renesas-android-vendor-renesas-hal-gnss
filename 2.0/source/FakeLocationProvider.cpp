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

#include "include/FakeLocationProvider.h"

namespace android::hardware::gnss::V2_0::renesas {

FakeLocationProvider::FakeLocationProvider(uint32_t interval) :
    LocationProviderBase(interval),
    mBuilder(std::make_unique<FakeLocationBuilder>()) {}

void FakeLocationProvider::Provide() {
    while (!mThreadExit) {
        LocationData data;
        auto error = mBuilder->Build(data);

        if (mEnabled && error == FLBError::SUCCESS) {
            if (mGnssCallback_1_1) {
                ALOGV("%s, Provide fake location callback", __func__);
                auto ret = mGnssCallback_1_1->gnssLocationCb(data.v1_0);

                if (!ret.isOk()) {
                    ALOGE("%s: Unable to invoke gnssLocationCb_1_1", __func__);
                }
            }
            if (mGnssCallback_2_0) {
                ALOGV("%s, Provide fake location callback", __func__);
                auto ret = mGnssCallback_2_0->gnssLocationCb_2_0(data);

                if (!ret.isOk()) {
                    ALOGE("%s: Unable to invoke gnssLocationCb_2_0", __func__);
                }
            }
        }
    }
}

} // namespace android::hardware::gnss::V2_0::renesas
