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
#pragma once

#include <atomic>
#include <thread>

#include <android/hardware/gnss/1.1/IGnssCallback.h>
#include <android/hardware/gnss/2.0/IGnssCallback.h>
#include "include/GnssInfoBuilder.h"

namespace android::hardware::gnss::V2_0::renesas {

using GnssCallback_1_1
    = ::android::sp<::android::hardware::gnss::V1_1::IGnssCallback>;
using GnssCallback_2_0
    = ::android::sp<::android::hardware::gnss::V2_0::IGnssCallback>;

class GnssInfoProvider {
public:
    GnssInfoProvider(uint32_t interval);
    virtual ~GnssInfoProvider() = default;

    void StartProviding();
    void StopProviding();
    void setCallback_1_1(GnssCallback_1_1& cb);
    void setCallback_2_0(GnssCallback_2_0& cb);
    void SetUpdateInterval(uint32_t newInterval);
    void SetEnabled(bool isEnabled);
protected:
    void Provide();
private:
    GnssInfoProvider(GnssInfoProvider&) = delete;
    GnssInfoProvider& operator=(const GnssInfoProvider&) = delete;
    ::android::hardware::gnss::V1_0::IGnssCallback::GnssSvStatus
        SvInfoV2_0T0_V1_1(SvInfoList v2_0);

    GnssCallback_1_1 mGnssCallback_1_1;
    GnssCallback_2_0 mGnssCallback_2_0;
    uint32_t mUpdateIntervalUs;
    std::unique_ptr<GnssInfoBuilder> mBuilder;
    std::thread mGnssSvInfoProvidingThread;
    std::atomic<bool> mThreadExit;
    std::atomic<bool> mEnabled;
};

} // namespace android::hardware::gnss::V2_0::renesas
