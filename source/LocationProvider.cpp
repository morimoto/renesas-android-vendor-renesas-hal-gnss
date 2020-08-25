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
#define LOG_TAG "GnssRenesasLocationProvider"
#include <log/log.h>

#include "include/LocationProvider.h"
#include "include/GnssMeasurementSync.h"

namespace android::hardware::gnss::V2_0::renesas {

LocationProvider::LocationProvider(uint32_t interval) :
    LocationProviderBase(interval),
    mBuilder(std::make_unique<LocationBuilder>()) {}

void LocationProvider::Provide() {
    ALOGV("%s", __func__);
    auto& syncInstance = GnssMeasurementSync::GetInstance();

    while (!mThreadExit) {
        LocationData data = {};
        auto error = mBuilder->Build(data);

        if (mEnabled && syncInstance.Ready()) {
            if (error == LBError::SUCCESS) {
                if (mGnssCallback_1_0) {
                    ALOGV("%s, Provide location callback_1_0", __PRETTY_FUNCTION__);
                    auto ret = mGnssCallback_1_0->gnssLocationCb(data.v1_0);

                    if (!ret.isOk()) {
                        ALOGE("%s: Unable to invoke gnssLocationCb_1_0", __func__);
                    }
                }
                if (mGnssCallback_1_1) {
                    ALOGV("%s, Provide location callback_1_1", __PRETTY_FUNCTION__);
                    auto ret = mGnssCallback_1_1->gnssLocationCb(data.v1_0);

                    if (!ret.isOk()) {
                        ALOGE("%s: Unable to invoke gnssLocationCb_1_1", __func__);
                    }
                }
                if (mGnssCallback_2_0) {
                    ALOGV("%s, Provide location callback_2_0", __PRETTY_FUNCTION__);
                    auto ret = mGnssCallback_2_0->gnssLocationCb_2_0(data);

                    if (!ret.isOk()) {
                        ALOGE("%s: Unable to invoke gnssLocationCb_2_0", __func__);
                    }
                }
            } else {
                ALOGE("%s: Unable to get valid location coordinates", __func__);
            }
        }

        usleep(mUpdateIntervalUs);
    }
}

} // namespace android::hardware::gnss::V2_0::renesas
