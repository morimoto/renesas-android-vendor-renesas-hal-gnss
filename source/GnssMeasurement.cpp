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
#define LOG_TAG "GnssRenesasMeasurement"

#include "include/GnssMeasurement.h"

#include <log/log.h>

#include "include/GnssMeasurementSync.h"

namespace android::hardware::gnss::V2_1::renesas {

using GnssMeasurementStatus
    = ::android::hardware::gnss::V1_0::IGnssMeasurement::GnssMeasurementStatus;

// required by cts testGnssMeasurementWhenNoLocation
static constexpr int8_t gnssMeasxExpectedBeforeLocation = 2;

// Methods from ::android::hardware::gnss::V1_0::IGnssMeasurement follow.
GnssMeasurement::~GnssMeasurement() {
    if (mProvider) {
        mProvider->StopProviding();
    }
}

Return<GnssMeasurementStatus>
GnssMeasurement::setCallback(const
        sp<::android::hardware::gnss::V1_0::IGnssMeasurementCallback>& callback) {

    ALOGE("%s", __func__);

    if (mGnssMeasurementsCbIface_1_0 != nullptr) {
        ALOGV("%s: GnssMeasurementCallback already inited", __func__);
        return GnssMeasurementStatus::ERROR_ALREADY_INIT;
    }

    mGnssMeasurementsCbIface_1_0 = callback;

    if (callback != nullptr) {
        auto& syncInstance = GnssMeasurementSync::GetInstance();
        syncInstance.SetEventsToWait(gnssMeasxExpectedBeforeLocation);

        if (!mProvider) {
            mProvider = std::make_unique<MeasurementProvider>();
            mProvider->StartProviding();
        }

        mProvider->setMeasxCallback_1_0(mGnssMeasurementsCbIface_1_0);
        mProvider->setEnabled(true);
    }

    return GnssMeasurementStatus::SUCCESS;
}

Return<void> GnssMeasurement::close() {
    if (mGnssMeasurementsCbIface_1_0 == nullptr
        && mGnssMeasurementsCbIface_1_1 == nullptr
        && mGnssMeasurementsCbIface_2_0 == nullptr) {
        return Void();
    }

    auto& syncInstance = GnssMeasurementSync::GetInstance();
    syncInstance.SetEventsToWait(0);

    if (nullptr != mProvider) {
        mProvider->setEnabled(false);
    }

    mGnssMeasurementsCbIface_1_0 = nullptr;
    mGnssMeasurementsCbIface_1_1 = nullptr;
    mGnssMeasurementsCbIface_2_0 = nullptr;
    return Void();
}

// Methods from ::android::hardware::gnss::V1_1::IGnssMeasurement follow.
Return<GnssMeasurementStatus>
GnssMeasurement::setCallback_1_1(const
    sp<::android::hardware::gnss::V1_1::IGnssMeasurementCallback>& callback,
    bool enableFullTracking) {
    ALOGV("%s", __func__);

    if (mGnssMeasurementsCbIface_1_1 != nullptr) {
        ALOGE("%s: GnssMeasurementCallback already inited", __func__);
        return GnssMeasurementStatus::ERROR_ALREADY_INIT;
    }

    if (enableFullTracking) {
        ALOGV("Full tracking enabled");
    }

    mGnssMeasurementsCbIface_1_1 = callback;

    if (callback != nullptr) {
        auto& syncInstance = GnssMeasurementSync::GetInstance();
        syncInstance.SetEventsToWait(gnssMeasxExpectedBeforeLocation);

        if (!mProvider) {
            mProvider = std::make_unique<MeasurementProvider>();
            mProvider->StartProviding();
        }

        mProvider->setMeasxCallback_1_1(mGnssMeasurementsCbIface_1_1);
        mProvider->setEnabled(true);
    }

    return GnssMeasurementStatus::SUCCESS;
}

Return<GnssMeasurementStatus>
GnssMeasurement::setCallback_2_0(const
    sp<::android::hardware::gnss::V2_0::IGnssMeasurementCallback>& callback,
    bool enableFullTracking) {
    ALOGV("%s", __func__);

    if (mGnssMeasurementsCbIface_2_0 != nullptr) {
        ALOGE("%s: GnssMeasurementCallback already inited", __func__);
        return GnssMeasurementStatus::ERROR_ALREADY_INIT;
    }

    if (enableFullTracking) {
        ALOGV("Full tracking enabled");
    }

    mGnssMeasurementsCbIface_2_0 = callback;

    if (callback != nullptr) {
        auto& syncInstance = GnssMeasurementSync::GetInstance();
        syncInstance.SetEventsToWait(gnssMeasxExpectedBeforeLocation);

        if (!mProvider) {
            mProvider = std::make_unique<MeasurementProvider>();
            mProvider->StartProviding();
        }

        mProvider->setMeasxCallback_2_0(mGnssMeasurementsCbIface_2_0);
        mProvider->setEnabled(true);
    }

    return GnssMeasurementStatus::SUCCESS;
}

// Methods from V2_1::IGnssMeasurement follow.
Return<V1_0::IGnssMeasurement::GnssMeasurementStatus> GnssMeasurement::setCallback_2_1(
        const sp<V2_1::IGnssMeasurementCallback>& callback, bool) {
    //TODO implement
    return V1_0::IGnssMeasurement::GnssMeasurementStatus::SUCCESS;
}

}  // namespace android::hardware::gnss::V2_1::renesas
