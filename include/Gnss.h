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

#include <android/hardware/gnss/2.0/IGnss.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <memory>

#include "include/GeneralManager.h"
#include "include/GnssConfiguration.h"
#include "include/GnssBatching.h"

class GeneralManager;

namespace android::hardware::gnss::V2_0::renesas {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;


class GnssImpl : public IGnss {
public:
    using IGnssCbV1_1 = ::android::hardware::gnss::V1_1::IGnssCallback;
    using IGnssCbV2_0 = ::android::hardware::gnss::V2_0::IGnssCallback;
    using IGnssMeasurementV1_1 =
                        ::android::hardware::gnss::V1_1::IGnssMeasurement;
    using IGnssMeasurementV2_0 =
                        ::android::hardware::gnss::V2_0::IGnssMeasurement;

    ::android::status_t SetGeneralManager(const std::shared_ptr<GeneralManager>
                                          genManager);
    Return<void> cleanup() override;
    Return<bool> setCallback_1_1(const sp<IGnssCbV1_1>& callback) override;
    Return<bool> setCallback_2_0(const sp<IGnssCbV2_0>& callback) override;

    Return<bool> start() override;
    Return<bool> stop() override;

    Return<sp<IGnssMeasurementV2_0>>
                                getExtensionGnssMeasurement_2_0() override;

private:
    std::shared_ptr<GeneralManager> mGeneralManager;
    sp<GnssConfiguration> mGnssConfig;
    sp<GnssBatching> mGnssBatching;

public:
    // Methods from ::android::hardware::gnss::V1_0::IGnss follow.
    Return<bool> setCallback(const
        sp<::android::hardware::gnss::V1_0::IGnssCallback>& cb) override;
    Return<bool> injectTime(int64_t timeMs, int64_t timeReferenceMs,
                            int32_t uncertaintyMs) override;
    Return<bool> injectLocation(double latitudeDegrees, double longitudeDegrees,
                                float accuracyMeters) override;
    Return<void> deleteAidingData(
        android::hardware::gnss::V1_0::IGnss::GnssAidingData aidingDataFlags)
        override;
    Return<bool> setPositionMode(
        android::hardware::gnss::V1_0::IGnss::GnssPositionMode mode,
        android::hardware::gnss::V1_0::IGnss::GnssPositionRecurrence
        recurrence, uint32_t minIntervalMs, uint32_t preferredAccuracyMeters,
        uint32_t preferredTimeMs) override;
    Return<sp<::android::hardware::gnss::V1_0::IAGnssRil>>
            getExtensionAGnssRil() override;
    Return<sp<::android::hardware::gnss::V1_0::IGnssGeofencing>>
            getExtensionGnssGeofencing() override;
    Return<sp<::android::hardware::gnss::V1_0::IAGnss>> getExtensionAGnss()
            override;
    Return<sp<::android::hardware::gnss::V1_0::IGnssNi>> getExtensionGnssNi()
            override;
    Return<sp<::android::hardware::gnss::V1_0::IGnssMeasurement>>
            getExtensionGnssMeasurement() override;
    Return<sp<::android::hardware::gnss::V1_0::IGnssNavigationMessage>>
            getExtensionGnssNavigationMessage() override;
    Return<sp<::android::hardware::gnss::V1_0::IGnssXtra>> getExtensionXtra()
            override;
    Return<sp<::android::hardware::gnss::V1_0::IGnssConfiguration>>
            getExtensionGnssConfiguration() override;
    Return<sp<::android::hardware::gnss::V1_0::IGnssDebug>>
            getExtensionGnssDebug() override;
    Return<sp<::android::hardware::gnss::V1_0::IGnssBatching>>
            getExtensionGnssBatching() override;

    // Methods from ::android::hardware::gnss::V1_1::IGnss follow.
    Return<bool> setPositionMode_1_1(
        ::android::hardware::gnss::V1_0::IGnss::GnssPositionMode mode,
        ::android::hardware::gnss::V1_0::IGnss::GnssPositionRecurrence 
        recurrence, uint32_t minIntervalMs, uint32_t preferredAccuracyMeters,
        uint32_t preferredTimeMs, bool lowPowerMode) override;
    Return<sp<::android::hardware::gnss::V1_1::IGnssConfiguration>>
            getExtensionGnssConfiguration_1_1() override;
    Return<sp<::android::hardware::gnss::V1_1::IGnssMeasurement>>
            getExtensionGnssMeasurement_1_1() override;
    Return<bool> injectBestLocation(
        const ::android::hardware::gnss::V1_0::GnssLocation& location) override;

    // Methods from ::android::hardware::gnss::V2_0::IGnss follow.
    Return<sp<::android::hardware::gnss::V2_0::IGnssConfiguration>>
            getExtensionGnssConfiguration_2_0() override;
    Return<sp<::android::hardware::gnss::V2_0::IGnssDebug>>
            getExtensionGnssDebug_2_0() override;
    Return<sp<::android::hardware::gnss::V2_0::IAGnss>> getExtensionAGnss_2_0()
            override;
    Return<sp<::android::hardware::gnss::V2_0::IAGnssRil>>
            getExtensionAGnssRil_2_0() override;
    Return<sp<::android::hardware::gnss::measurement_corrections::V1_0::IMeasurementCorrections>>
            getExtensionMeasurementCorrections() override;
    Return<sp<::android::hardware::gnss::visibility_control::V1_0::IGnssVisibilityControl>>
            getExtensionVisibilityControl() override;
    Return<sp<::android::hardware::gnss::V2_0::IGnssBatching>>
            getExtensionGnssBatching_2_0() override;
    Return<bool> injectBestLocation_2_0(
        const ::android::hardware::gnss::V2_0::GnssLocation&
        location) override;
};

}  // namespace android::hardware::gnss::V2_0::renesas
