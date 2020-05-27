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
#pragma once

#include <thread>
#include <atomic>
#include <functional>

#include "include/ILocationProvider.h"

namespace android::hardware::gnss::V2_0::renesas {

using GnssCallback_1_0 = android::sp<android::hardware::gnss::V1_0::IGnssCallback>;
using GnssCallback_1_1 = android::sp<android::hardware::gnss::V1_1::IGnssCallback>;
using GnssCallback_2_0 = android::sp<android::hardware::gnss::V2_0::IGnssCallback>;

class LocationProviderBase : public ILocationProvider {
public:
    LocationProviderBase(uint32_t interval);
    ~LocationProviderBase() override;
    LPError StartProviding() override;
    LPError StopProviding() override;
    void SetUpdateInterval(uint32_t newInterval) override;
    void setCallback_1_0(GnssCallback_1_0& cb) override;
    void setCallback_1_1(GnssCallback_1_1& cb) override;
    void setCallback_2_0(GnssCallback_2_0& cb) override;
    void SetEnabled(bool isEnabled) override;
protected:
    /*!
     * \brief Provide
     */
    virtual void Provide() = 0;

    std::atomic<bool> mThreadExit;
    std::thread mGnssLocationThread;
    GnssCallback_1_0 mGnssCallback_1_0;
    GnssCallback_1_1 mGnssCallback_1_1;
    GnssCallback_2_0 mGnssCallback_2_0;
    std::atomic<bool> mEnabled;
    uint32_t mUpdateIntervalUs;
private:
    LocationProviderBase(LocationProviderBase&) = delete;
    LocationProviderBase& operator=(const LocationProviderBase&) = delete;
};

} // namespace android::hardware::gnss::V2_0::renesas
