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

#ifndef GNSSINFOPROVIDER_H
#define GNSSINFOPROVIDER_H

#include <unordered_map>

#include <GnssInfoBuilder.h>

namespace android::hardware::gnss::V2_1::renesas {

using GnssCallback_1_0
    = ::android::sp<::android::hardware::gnss::V1_0::IGnssCallback>;
using GnssCallback_1_1
    = ::android::sp<::android::hardware::gnss::V1_1::IGnssCallback>;
using GnssCallback_2_0
    = ::android::sp<::android::hardware::gnss::V2_0::IGnssCallback>;
using GnssCallback_2_1
    = ::android::sp<::android::hardware::gnss::V2_1::IGnssCallback>;
using GnssSvStatus_1_0
    = ::android::hardware::gnss::V1_0::IGnssCallback::GnssSvStatus;

class GnssInfoProvider {
public:
    GnssInfoProvider(uint32_t interval);
    virtual ~GnssInfoProvider() = default;

    void StartProviding();
    void StopProviding();
    void setCallback_1_0(GnssCallback_1_0& cb);
    void setCallback_1_1(GnssCallback_1_1& cb);
    void setCallback_2_0(GnssCallback_2_0& cb);
    void setCallback_2_1(GnssCallback_2_1& cb);
    void SetUpdateInterval(uint32_t newInterval);
    void SetEnabled(bool isEnabled);
protected:
    void Provide();
private:
    GnssInfoProvider(GnssInfoProvider&) = delete;
    GnssInfoProvider& operator=(const GnssInfoProvider&) = delete;
    ::android::hardware::gnss::V1_0::IGnssCallback::GnssSvStatus
        SvInfoV2_1To_V1_0(const SvInfoList& v2_1);
    std::vector<::android::hardware::gnss::V2_0::IGnssCallback::GnssSvInfo>
        SvInfoV2_1To_V2_0(const SvInfoList& v2_1);

    GnssCallback_1_0 mGnssCallback_1_0;
    GnssCallback_1_1 mGnssCallback_1_1;
    GnssCallback_2_0 mGnssCallback_2_0;
    GnssCallback_2_1 mGnssCallback_2_1;
    uint32_t mUpdateIntervalUs;
    std::unique_ptr<GnssInfoBuilder> mBuilder;
    std::thread mGnssSvInfoProvidingThread;
    std::atomic<bool> mThreadExit;
    std::atomic<bool> mEnabled;
    std::unordered_map<std::string, std::function<void(const SvInfoList&)>> mProviders;
};

} // namespace android::hardware::gnss::V2_1::renesas

#endif // GNSSINFOPROVIDER_H
