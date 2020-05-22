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
#define LOG_TAG "GnssRenesasInfoProvider"
#include <log/log.h>

#include "include/GnssInfoProvider.h"

namespace android::hardware::gnss::V2_0::renesas {

GnssInfoProvider::GnssInfoProvider(uint32_t interval) :
    mUpdateIntervalUs(interval),
    mBuilder(std::make_unique<GnssInfoBuilder>()) {}

void GnssInfoProvider::StartProviding() {
    mThreadExit = false;

    if (!mGnssSvInfoProvidingThread.joinable()) {
        mGnssSvInfoProvidingThread =
            std::thread(&GnssInfoProvider::Provide, this);
    }
}

void GnssInfoProvider::StopProviding() {
    mThreadExit = true;

    if (mGnssSvInfoProvidingThread.joinable()) {
        mGnssSvInfoProvidingThread.join();
    }
}

void GnssInfoProvider::Provide() {
    ALOGV("%s", __func__);

    while (!mThreadExit) {
        SvInfoList outList;
        auto error = mBuilder->Build(outList);

        if (mEnabled && error == IBError::SUCCESS) {
            if (mGnssCallback_1_1) {
                auto ret = mGnssCallback_1_1->gnssSvStatusCb(SvInfoV2_0To_V1_1(outList));

                if (!ret.isOk()) {
                    ALOGE("%s: Unable to invoke gnssSvStatusCb_1_1", __func__);
                }
            }
            if (mGnssCallback_2_0) {
                auto ret = mGnssCallback_2_0->gnssSvStatusCb_2_0(outList);

                if (!ret.isOk()) {
                    ALOGE("%s: Unable to invoke gnssSvStatusCb_2_0", __func__);
                }
            }
        }

        usleep(mUpdateIntervalUs);
    }
}

void GnssInfoProvider::SetUpdateInterval(uint32_t newInterval) {
    mUpdateIntervalUs = newInterval;
}

void GnssInfoProvider::setCallback_1_1(GnssCallback_1_1& cb) {
    mGnssCallback_1_1 = cb;
}

void GnssInfoProvider::setCallback_2_0(GnssCallback_2_0& cb) {
    mGnssCallback_2_0 = cb;
}

void GnssInfoProvider::SetEnabled(bool isEnabled) {
    mEnabled = isEnabled;
}

GnssSvStatus_1_0 GnssInfoProvider::SvInfoV2_0To_V1_1(SvInfoList v2_0) {
    GnssSvStatus_1_0 v1_1;

    v1_1.numSvs = v2_0.size();
    for (size_t i = 0; i < v2_0.size(); i++) {
        v1_1.gnssSvList[i] = v2_0[i].v1_0;
    }

    return v1_1;
}

}