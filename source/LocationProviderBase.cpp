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
#define LOG_TAG "GnssRenesasLocationProviderBase"

#include <LocationProviderBase.h>

#include <log/log.h>

namespace android::hardware::gnss::V2_1::renesas {

LocationProviderBase::LocationProviderBase(uint32_t interval):
    mUpdateIntervalUs(interval) {
}

LocationProviderBase::~LocationProviderBase() {
    StopProviding();
}

LPError LocationProviderBase::StartProviding() {
    mThreadExit = false;

    if (!mGnssLocationThread.joinable()) {
        mGnssLocationThread = std::thread(&LocationProviderBase::Provide, this);
    }

    return LPError::SUCCESS;
}

LPError LocationProviderBase::StopProviding() {
    mThreadExit = true;

    if (mGnssLocationThread.joinable()) {
        mGnssLocationThread.join();
    }

    return LPError::SUCCESS;
}

void LocationProviderBase::SetUpdateInterval(uint32_t newInterval) {
    mUpdateIntervalUs = newInterval;
}

void LocationProviderBase::setCallback_1_0(GnssCallback_1_0& cb) {
    static const std::string providerName = {"GnssCallback_1_0"};

    mGnssCallback_1_0 = cb;
    mProviders[providerName] = [this](const LocationData& location) {
        if (mGnssCallback_1_0) {
            ALOGV("%s, Provide location callback_1_0", __PRETTY_FUNCTION__);
            auto ret = mGnssCallback_1_0->gnssLocationCb(location.v1_0);

            if (!ret.isOk()) {
                ALOGE("%s: Unable to invoke gnssLocationCb_1_0", __func__);
            }
        }
    };
}

void LocationProviderBase::setCallback_1_1(GnssCallback_1_1& cb) {
    static const std::string providerName = {"GnssCallback_1_1"};

    mGnssCallback_1_1 = cb;
    mProviders[providerName] = [this](const LocationData& location) {
        if (mGnssCallback_1_1) {
            ALOGV("%s, Provide location callback_1_1", __PRETTY_FUNCTION__);
            auto ret = mGnssCallback_1_1->gnssLocationCb(location.v1_0);

            if (!ret.isOk()) {
                ALOGE("%s: Unable to invoke gnssLocationCb_1_1", __func__);
            }
        }
    };
}

void LocationProviderBase::setCallback_2_0(GnssCallback_2_0& cb) {
    static const std::string providerName = {"GnssCallback_2_0"};

    mGnssCallback_2_0 = cb;
    mProviders[providerName] = [this](const LocationData& location) {
        if (mGnssCallback_2_0) {
            ALOGV("%s, Provide location callback_2_0", __PRETTY_FUNCTION__);
            auto ret = mGnssCallback_2_0->gnssLocationCb_2_0(location);

            if (!ret.isOk()) {
                ALOGE("%s: Unable to invoke gnssLocationCb_2_0", __func__);
                return;
            }

            if (mGnssVisibilityControl) {
                mGnssVisibilityControl->sendNfwNotificationMsg(
                    mGnssCallback_2_0->descriptor);
            }
        }
    };
}

void LocationProviderBase::setCallback_2_1(GnssCallback_2_1& cb) {
    static const std::string providerName = {"GnssCallback_2_1"};

    mGnssCallback_2_1 = cb;
    mProviders[providerName] = [this](const LocationData& location) {
        if (mGnssCallback_2_1) {
            ALOGV("%s, Provide location callback_2_1", __PRETTY_FUNCTION__);
            auto ret = mGnssCallback_2_1->gnssLocationCb_2_0(location);

            if (!ret.isOk()) {
                ALOGE("%s: Unable to invoke gnssLocationCb_2_1", __func__);
                return;
            }

            if (mGnssVisibilityControl) {
                mGnssVisibilityControl->sendNfwNotificationMsg(
                    mGnssCallback_2_1->descriptor);
            }
        }
    };
}

void LocationProviderBase::CallProviders(const LocationData& location) {
    for (auto provider : mProviders) {
        provider.second(location);
    }
}

void LocationProviderBase::SetEnabled(bool isEnabled) {
    mEnabled = isEnabled;
}

void LocationProviderBase::setGnssVisibilityControl(
        sp<GnssVisibilityControlV1_0>& gnssVisibilityControl) {
    mGnssVisibilityControl = gnssVisibilityControl;
}

}  // namespace android::hardware::gnss::V2_1::renesas
