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
#include <unordered_map>

#include <MeasurementBuilder.h>

namespace android::hardware::gnss::V2_1::renesas {

/**
 * @brief IGnssMeasxCb 1.0
 */
using IGnssMeasxCb_1_0 = ::android::sp<android::hardware::gnss::V1_0::IGnssMeasurementCallback>;

/**
 * @brief IGnssMeasxCb 1.1
 */
using IGnssMeasxCb_1_1 = ::android::sp<android::hardware::gnss::V1_1::IGnssMeasurementCallback>;

/**
 * @brief IGnssMeasxCb 2.0
 */
using IGnssMeasxCb_2_0 = ::android::sp<android::hardware::gnss::V2_0::IGnssMeasurementCallback>;

/**
 * @brief IGnssMeasxCb 2.1
 */
using IGnssMeasxCb_2_1 = ::android::sp<android::hardware::gnss::V2_1::IGnssMeasurementCallback>;

/**
 * @brief GnssData 1.0
 */
using GnssData_1_0 = android::hardware::gnss::V1_0::IGnssMeasurementCallback::GnssData;

/**
 * @brief GnssData 1.1
 */
using GnssData_1_1 = android::hardware::gnss::V1_1::IGnssMeasurementCallback::GnssData;

/**
 * @brief GnssData 2.0
 */
using GnssData_2_0 = android::hardware::gnss::V2_0::IGnssMeasurementCallback::GnssData;

/**
 * @brief GnssData 2.1
 */
using GnssData_2_1 = android::hardware::gnss::V2_1::IGnssMeasurementCallback::GnssData;

/**
 * @brief Measurement Provider implementation
 */
class MeasurementProvider {
public:
    /**
     * @brief Construct a new Measurement Provider object
     */
    MeasurementProvider();

    /**
     * @brief Destroy the Measurement Provider object
     */
    ~MeasurementProvider() = default;

    /**
     * @brief Start Providing
     */
    void StartProviding();

    /**
     * @brief Stop Providing
     */
    void StopProviding();

    /**
     * @brief Set the MeasxCallback 1.0 object
     *
     * @param measxCb
     */
    void setMeasxCallback_1_0(IGnssMeasxCb_1_0 measxCb);

    /**
     * @brief Set the MeasxCallback 1.1 object
     *
     * @param measxCb
     */
    void setMeasxCallback_1_1(IGnssMeasxCb_1_1 measxCb);

    /**
     * @brief Set the MeasxCallback 2.0 object
     *
     * @param measxCb
     */
    void setMeasxCallback_2_0(IGnssMeasxCb_2_0 measxCb);

    /**
     * @brief Set the MeasxCallback 2.1 object
     *
     * @param measxCb
     */
    void setMeasxCallback_2_1(IGnssMeasxCb_2_1 measxCb);

    /**
     * @brief Set the Enabled
     *
     * @param isEnabled
     */
    void setEnabled(bool isEnabled);

private:
    MeasurementProvider(MeasurementProvider&) = delete;
    MeasurementProvider& operator=(const MeasurementProvider&) = delete;
    void Provide();
    GnssData_1_0  DataV2_1ToDataV1_0(const GnssData_2_1& v2_1);
    GnssData_1_1  DataV2_1ToDataV1_1(const GnssData_2_1& v2_1);
    GnssData_2_0  DataV2_1ToDataV2_0(const GnssData_2_1& v2_1);

    std::unique_ptr<MeasurementBuilder> mBuilder;
    std::thread mGnssMeasurementsCallbackThread;
    std::atomic<bool> mThreadExit;
    IGnssMeasxCb_1_0 mGnssMeasurementsCbIface_1_0;
    IGnssMeasxCb_1_1 mGnssMeasurementsCbIface_1_1;
    IGnssMeasxCb_2_0 mGnssMeasurementsCbIface_2_0;
    IGnssMeasxCb_2_1 mGnssMeasurementsCbIface_2_1;
    std::mutex mCallbackMutex;
    std::condition_variable mCallbackCond;
    std::atomic<bool> mEnabled;
    std::unordered_map<std::string, std::function<void(const GnssData_2_1&)>> mProviders;
};

} //namespace android::hardware::gnss::V2_1::renesas

#endif // MEASUREMENTPROVIDER_H
