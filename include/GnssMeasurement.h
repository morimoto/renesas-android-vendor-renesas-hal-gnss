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

#ifndef GNSSMEASUREMENT_H
#define GNSSMEASUREMENT_H

#include <MeasurementProvider.h>

namespace android::hardware::gnss::V2_1::renesas {

/**
 * @brief IGnssMeasurementCallback_1_0
 */
using IGnssMeasurementCallback_1_0
    = android::hardware::gnss::V1_0::IGnssMeasurementCallback;
/**
 * @brief IGnssMeasurementCallback_1_1
 */
using IGnssMeasurementCallback_1_1
    = android::hardware::gnss::V1_1::IGnssMeasurementCallback;
/**
 * @brief IGnssMeasurementCallback_2_0
 */
using IGnssMeasurementCallback_2_0
    = android::hardware::gnss::V2_0::IGnssMeasurementCallback;
/**
 * @brief IGnssMeasurementCallback_2_1
 */
using IGnssMeasurementCallback_2_1
    = android::hardware::gnss::V2_1::IGnssMeasurementCallback;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

/**
 * @brief Gnss measurement class
 */
class GnssMeasurement : public android::hardware::gnss::V2_1::IGnssMeasurement {
public:
    /**
     * @brief Construct a new Gnss Measurement object
     */
    GnssMeasurement() {};
    ~GnssMeasurement() override;
    // Methods from ::android::hardware::gnss::V1_0::IGnssMeasurement follow.
    Return
    <::android::hardware::gnss::V1_0::IGnssMeasurement::GnssMeasurementStatus>
    setCallback(const
        sp<::android::hardware::gnss::V1_0::IGnssMeasurementCallback>&
        callback) override;
    Return<void> close() override;

    // Methods from ::android::hardware::gnss::V1_1::IGnssMeasurement follow.
    Return
    <::android::hardware::gnss::V1_0::IGnssMeasurement::GnssMeasurementStatus>
    setCallback_1_1(const
        sp<IGnssMeasurementCallback_1_1>&
        callback, bool enableFullTracking) override;

    // Methods from ::android::hardware::gnss::V2_0::IGnssMeasurement follow.
    Return
    <::android::hardware::gnss::V1_0::IGnssMeasurement::GnssMeasurementStatus>
    setCallback_2_0(const
        sp<IGnssMeasurementCallback_2_0>&
        callback, bool enableFullTracking) override;

    // Methods from V2_1::IGnssMeasurement follow.
    Return
    <V1_0::IGnssMeasurement::GnssMeasurementStatus>
    setCallback_2_1(const
        sp<V2_1::IGnssMeasurementCallback>&
        callback, bool enableFullTracking) override;
private:
    std::unique_ptr<MeasurementProvider> mProvider;
    sp<IGnssMeasurementCallback_1_0> mGnssMeasurementsCbIface_1_0;
    sp<IGnssMeasurementCallback_1_1> mGnssMeasurementsCbIface_1_1;
    sp<IGnssMeasurementCallback_2_0> mGnssMeasurementsCbIface_2_0;
    sp<IGnssMeasurementCallback_2_1> mGnssMeasurementsCbIface_2_1;
};

}  // namespace android::hardware::gnss::V2_1::renesas

#endif // GNSSMEASUREMENT_H
