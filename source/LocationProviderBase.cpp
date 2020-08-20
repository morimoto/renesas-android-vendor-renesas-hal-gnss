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

#include "include/LocationProviderBase.h"

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
    mGnssCallback_1_0 = cb;
}

void LocationProviderBase::setCallback_1_1(GnssCallback_1_1& cb) {
    mGnssCallback_1_1 = cb;
}

void LocationProviderBase::setCallback_2_0(GnssCallback_2_0& cb) {
    mGnssCallback_2_0 = cb;
}

void LocationProviderBase::setCallback_2_1(GnssCallback_2_1& cb) {
    mGnssCallback_2_1 = cb;
}

void LocationProviderBase::SetEnabled(bool isEnabled) {
    mEnabled = isEnabled;
}

} // namespace android::hardware::gnss::V2_1::renesas
