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

#include <GnssInfoProvider.h>

#include <log/log.h>

namespace android::hardware::gnss::V2_1::renesas {

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
            for (auto provider : mProviders) {
                provider.second(outList);
            }
        }

        usleep(mUpdateIntervalUs);
    }
}

void GnssInfoProvider::SetUpdateInterval(uint32_t newInterval) {
    mUpdateIntervalUs = newInterval;
}

void GnssInfoProvider::setCallback_1_0(GnssCallback_1_0& cb) {
    static const std::string providerName = {"GnssCallback_1_0"};

    mGnssCallback_1_0 = cb;
    mProviders[providerName] = [this](const SvInfoList& data) {
        if (mGnssCallback_1_0) {
            auto ret = mGnssCallback_1_0->gnssSvStatusCb(
                SvInfoV2_1To_V1_0(data));

            if (!ret.isOk()) {
                ALOGE("%s: Unable to invoke gnssSvStatusCb_1_0", __func__);
            }
        }
    };
}

void GnssInfoProvider::setCallback_1_1(GnssCallback_1_1& cb) {
    static const std::string providerName = {"GnssCallback_1_1"};

    mGnssCallback_1_1 = cb;
    mProviders[providerName] = [this](const SvInfoList& data) {
        if (mGnssCallback_1_1) {
            auto ret = mGnssCallback_1_1->gnssSvStatusCb(
                SvInfoV2_1To_V1_0(data));

            if (!ret.isOk()) {
                ALOGE("%s: Unable to invoke gnssSvStatusCb_1_1", __func__);
            }
        }
    };
}

void GnssInfoProvider::setCallback_2_0(GnssCallback_2_0& cb) {
    static const std::string providerName = {"GnssCallback_2_0"};

    mGnssCallback_2_0 = cb;
    mProviders[providerName] = [this](const SvInfoList& data) {
        if (mGnssCallback_2_0) {
            auto ret = mGnssCallback_2_0->gnssSvStatusCb_2_0(
                SvInfoV2_1To_V2_0(data));

            if (!ret.isOk()) {
                ALOGE("%s: Unable to invoke gnssSvStatusCb_2_0", __func__);
            }
        }
    };
}

void GnssInfoProvider::setCallback_2_1(GnssCallback_2_1& cb) {
    static const std::string providerName = {"GnssCallback_2_1"};

    mGnssCallback_2_1 = cb;
    mProviders[providerName] = [this](const SvInfoList& data) {
        if (mGnssCallback_2_1) {
            auto ret = mGnssCallback_2_1->gnssSvStatusCb_2_1(data);

            if (!ret.isOk()) {
                ALOGE("%s: Unable to invoke gnssSvStatusCb_2_1", __func__);
            }
        }
    };
}

void GnssInfoProvider::SetEnabled(bool isEnabled) {
    mEnabled = isEnabled;
}
::android::hardware::gnss::V1_0::IGnssCallback::GnssSvStatus
    GnssInfoProvider::SvInfoV2_1To_V1_0(const SvInfoList& v2_1) {
    ::android::hardware::gnss::V1_0::IGnssCallback::GnssSvStatus v1_0;
    v1_0.numSvs = v2_1.size();
    for (size_t i = 0; i < v2_1.size(); i++) {
        v1_0.gnssSvList[i] = v2_1[i].v2_0.v1_0;
    }
    return v1_0;
}

std::vector<::android::hardware::gnss::V2_0::IGnssCallback::GnssSvInfo>
    GnssInfoProvider::SvInfoV2_1To_V2_0(const SvInfoList& v2_1) {
    std::vector<::android::hardware::gnss::V2_0::IGnssCallback::GnssSvInfo>
        v2_0(v2_1.size());
    for (size_t i = 0; i < v2_1.size(); i++) {
        v2_0.push_back(v2_1[i].v2_0);
    }
    return v2_0;
}

}
