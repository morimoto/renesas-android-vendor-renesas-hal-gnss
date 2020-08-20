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

#ifndef MEASUREMENTPROVIDER_H
#define MEASUREMENTPROVIDER_H

#include <thread>

#include "include/MeasurementBuilder.h"

namespace android::hardware::gnss::V2_1::renesas {

using IGnssMeasxCb_1_0 = ::android::sp<android::hardware::gnss::V1_0::IGnssMeasurementCallback>;
using IGnssMeasxCb_1_1 = ::android::sp<android::hardware::gnss::V1_1::IGnssMeasurementCallback>;
using IGnssMeasxCb_2_0 = ::android::sp<android::hardware::gnss::V2_0::IGnssMeasurementCallback>;
using GnssData_1_0 = android::hardware::gnss::V1_0::IGnssMeasurementCallback::GnssData;
using GnssData_1_1 = android::hardware::gnss::V1_1::IGnssMeasurementCallback::GnssData;
using GnssData_2_0 = android::hardware::gnss::V2_0::IGnssMeasurementCallback::GnssData;

class MeasurementProvider {
public:
    MeasurementProvider();
    ~MeasurementProvider() = default;

    void StartProviding();
    void StopProviding();
    void setMeasxCallback_1_0(IGnssMeasxCb_1_0 measxCb);
    void setMeasxCallback_1_1(IGnssMeasxCb_1_1 measxCb);
    void setMeasxCallback_2_0(IGnssMeasxCb_2_0 measxCb);
    void setEnabled(bool isEnabled);

private:
    MeasurementProvider(MeasurementProvider&) = delete;
    MeasurementProvider& operator=(const MeasurementProvider&) = delete;
    void Provide();
    GnssData_1_0  DataV2_0ToDataV1_0(const GnssData& v2_0);
    GnssData_1_1  DataV2_0ToDataV1_1(const GnssData& v2_0);

    std::unique_ptr<MeasurementBuilder> mBuilder;
    std::thread mGnssMeasurementsCallbackThread;
    std::atomic<bool> mThreadExit;
    IGnssMeasxCb_1_0 mGnssMeasurementsCbIface_1_0;
    IGnssMeasxCb_1_1 mGnssMeasurementsCbIface_1_1;
    IGnssMeasxCb_2_0 mGnssMeasurementsCbIface_2_0;
    std::mutex mCallbackMutex;
    std::condition_variable mCallbackCond;
    std::atomic<bool> mEnabled;
};

} //namespace android::hardware::gnss::V2_1::renesas

#endif // MEASUREMENTPROVIDER_H
