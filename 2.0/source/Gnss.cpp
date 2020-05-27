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
#define LOG_NDEBUG 1
#define LOG_TAG "GnssRenesasImpl"
#include "include/Gnss.h"

namespace android {
namespace hardware {
namespace gnss {
namespace V2_0 {
namespace renesas {

::android::status_t GnssImpl::SetGeneralManager(const
        std::shared_ptr<GeneralManager> genManager) {
    mGeneralManager = genManager;
    return ::android::OK;
}

Return<bool> GnssImpl::setCallback(const
            sp<::android::hardware::gnss::V1_0::IGnssCallback>& cb) {
    ALOGV("%s", __PRETTY_FUNCTION__);

    if (mGeneralManager) {
        return (GMError::SUCCESS == mGeneralManager->SetCallbackV1_0(cb));
    }

    return false;
}

Return<bool> GnssImpl::start() {
    ALOGV("%s", __PRETTY_FUNCTION__);

    if (mGeneralManager) {
        return (GMError::SUCCESS == mGeneralManager->GnssStart());
    }

    return false;
}

Return<bool> GnssImpl::stop() {
    ALOGV("%s", __PRETTY_FUNCTION__);

    if (mGeneralManager) {
        return (GMError::SUCCESS == mGeneralManager->GnssStop());
    }

    return false;
}

Return<void> GnssImpl::cleanup() {
    ALOGV("%s", __PRETTY_FUNCTION__);

    if (mGeneralManager) {
        mGeneralManager->CleanUpCb();
    }

    return Void();
}

Return<bool> GnssImpl::injectTime([[maybe_unused]] int64_t
                    timeMs, [[maybe_unused]] int64_t
                    timeReferenceMs, [[maybe_unused]] int32_t uncertaintyMs) {
    // TODO implement
    return true;
}

Return<bool> GnssImpl::injectLocation([[maybe_unused]] double
                                      latitudeDegrees, [[maybe_unused]] double
                                      longitudeDegrees,
                                      [[maybe_unused]] float accuracyMeters) {
    // TODO implement
    return true;
}

Return<void>
GnssImpl::deleteAidingData([[maybe_unused]] 
    ::android::hardware::gnss::V1_0::IGnss::GnssAidingData aidingDataFlags) {
    // TODO implement
    return Void();
}

Return<bool>GnssImpl::setPositionMode(
    [[maybe_unused]]
        ::android::hardware::gnss::V1_0::IGnss::GnssPositionMode mode,
    [[maybe_unused]]
        ::android::hardware::gnss::V1_0::IGnss::GnssPositionRecurrence
        recurrence,
    [[maybe_unused]] uint32_t minIntervalMs,
    [[maybe_unused]]uint32_t preferredAccuracyMeters,
    [[maybe_unused]] uint32_t preferredTimeMs) {
    // TODO implement
    return true;
}

Return<sp<::android::hardware::gnss::V1_0::IAGnssRil>>
GnssImpl::getExtensionAGnssRil() {
    // TODO implement
    return ::android::sp<::android::hardware::gnss::V1_0::IAGnssRil> {};
}

Return<sp<::android::hardware::gnss::V1_0::IGnssGeofencing>>
GnssImpl::getExtensionGnssGeofencing() {
    // TODO implement
    return ::android::sp<::android::hardware::gnss::V1_0::IGnssGeofencing> {};
}

Return<sp<::android::hardware::gnss::V1_0::IAGnss>>
GnssImpl::getExtensionAGnss() {
    // TODO implement
    return ::android::sp<::android::hardware::gnss::V1_0::IAGnss> {};
}

Return<sp<::android::hardware::gnss::V1_0::IGnssNi>>
GnssImpl::getExtensionGnssNi() {
    // TODO implement
    return ::android::sp<::android::hardware::gnss::V1_0::IGnssNi> {};
}

Return<sp<::android::hardware::gnss::V1_0::IGnssMeasurement>>
GnssImpl::getExtensionGnssMeasurement() {
    if (!mGeneralManager) {
        ALOGE("%s: No general manager!", __func__);
        return nullptr;
    }

    return mGeneralManager->getExtensionGnssMeasurement_v2_0(); {};
}

Return<sp<::android::hardware::gnss::V1_0::IGnssNavigationMessage>>
GnssImpl::getExtensionGnssNavigationMessage() {
    // TODO implement
    return
    ::android::sp<::android::hardware::gnss::V1_0::IGnssNavigationMessage> {};
}

Return<sp<::android::hardware::gnss::V1_0::IGnssXtra>>
GnssImpl::getExtensionXtra() {
    // TODO implement
    return ::android::sp<::android::hardware::gnss::V1_0::IGnssXtra> {};
}

Return<sp<::android::hardware::gnss::V1_0::IGnssConfiguration>>
GnssImpl::getExtensionGnssConfiguration() {
    // TODO implement
    return
        ::android::sp<::android::hardware::gnss::V1_0::IGnssConfiguration> {};
}

Return<sp<::android::hardware::gnss::V1_0::IGnssDebug>>
GnssImpl::getExtensionGnssDebug() {
    // TODO implement
    return ::android::sp<::android::hardware::gnss::V1_0::IGnssDebug> {};
}

Return<sp<::android::hardware::gnss::V1_0::IGnssBatching>>
GnssImpl::getExtensionGnssBatching() {
    // TODO implement
    return ::android::sp<::android::hardware::gnss::V1_0::IGnssBatching> {};
}


Return<bool> GnssImpl::setCallback_1_1(const
            sp<::android::hardware::gnss::V1_1::IGnssCallback>& cb) {
    ALOGV("%s", __PRETTY_FUNCTION__);

    if (mGeneralManager) {
        return (GMError::SUCCESS == mGeneralManager->SetCallbackV1_1(cb));
    }

    return false;
}

Return<bool> GnssImpl::setPositionMode_1_1(
    ::android::hardware::gnss::V1_0::IGnss::GnssPositionMode mode,
    ::android::hardware::gnss::V1_0::IGnss::GnssPositionRecurrence recurrence,
    uint32_t minIntervalMs, uint32_t preferredAccuracyMeters,
    uint32_t preferredTimeMs, bool lowPowerMode) {
    ALOGV("%s: mode=%d, recurrence=%d, minIntervalMs=%d,"
          "preferredAccuracyMeters=%d, preferredTimeMs=%d, lowPowerMode=%d",
          __func__, (int)mode, recurrence, minIntervalMs,
          preferredAccuracyMeters, preferredTimeMs, lowPowerMode);
    mGeneralManager->setUpdatePeriod(minIntervalMs);
    return true;
}

Return<sp<::android::hardware::gnss::V1_1::IGnssConfiguration>>
GnssImpl::getExtensionGnssConfiguration_1_1() {
    // TODO implement
    return
        ::android::sp<::android::hardware::gnss::V1_1::IGnssConfiguration> {};
}

Return<sp<::android::hardware::gnss::V1_1::IGnssMeasurement>>
GnssImpl::getExtensionGnssMeasurement_1_1() {
    if (!mGeneralManager) {
        ALOGE("%s: No general manager!", __func__);
        return nullptr;
    }

    return mGeneralManager->getExtensionGnssMeasurement_v2_0();
}

Return<bool> GnssImpl::injectBestLocation(
    const ::android::hardware::gnss::V1_0::GnssLocation& location) {
    ALOGV("latitude=%lf degrees,longitude=%lf degrees,altitude=%lf meters",
        location.latitudeDegrees, location.longitudeDegrees,
        location.altitudeMeters);
    return true;
}


Return<bool> GnssImpl::setCallback_2_0(const
                    sp<::android::hardware::gnss::V2_0::IGnssCallback>& cb) {
    ALOGV("%s", __PRETTY_FUNCTION__);

    if (mGeneralManager) {
        return (GMError::SUCCESS == mGeneralManager->SetCallbackV2_0(cb));
    }

    return false;
}

Return<sp<::android::hardware::gnss::V2_0::IGnssConfiguration>>
GnssImpl::getExtensionGnssConfiguration_2_0() {
    if (!mGnssConfig) {
        mGnssConfig = new GnssConfiguration();
    }

    return mGnssConfig;
}

Return<sp<::android::hardware::gnss::V2_0::IGnssDebug>>
GnssImpl::getExtensionGnssDebug_2_0() {
    // TODO implement
    return ::android::sp<::android::hardware::gnss::V2_0::IGnssDebug> {};
}

Return<sp<::android::hardware::gnss::V2_0::IAGnss>>
GnssImpl::getExtensionAGnss_2_0() {
    // TODO implement
    return ::android::sp<::android::hardware::gnss::V2_0::IAGnss> {};
}

Return<sp<::android::hardware::gnss::V2_0::IAGnssRil>>
GnssImpl::getExtensionAGnssRil_2_0() {
    // TODO implement
    return ::android::sp<::android::hardware::gnss::V2_0::IAGnssRil> {};
}

Return<sp<::android::hardware::gnss::V2_0::IGnssMeasurement>>
GnssImpl::getExtensionGnssMeasurement_2_0() {
    if (!mGeneralManager) {
        ALOGE("%s: No general manager!", __func__);
        return nullptr;
    }

    return mGeneralManager->getExtensionGnssMeasurement_v2_0();
}

Return<sp<::android::hardware::gnss::measurement_corrections::V1_0::IMeasurementCorrections>>
GnssImpl::getExtensionMeasurementCorrections() {
    // TODO implement
    return ::android::sp<::android::hardware::gnss::measurement_corrections::V1_0::IMeasurementCorrections> {};
}

Return<sp<::android::hardware::gnss::visibility_control::V1_0::IGnssVisibilityControl>>
GnssImpl::getExtensionVisibilityControl() {
    // TODO implement
    return ::android::sp<::android::hardware::gnss::visibility_control::V1_0::IGnssVisibilityControl> {};
}

Return<sp<::android::hardware::gnss::V2_0::IGnssBatching>>
GnssImpl::getExtensionGnssBatching_2_0() {
    if (!mGnssBatching) {
        mGnssBatching = new GnssBatching();
    }

    return mGnssBatching;
}

Return<bool> GnssImpl::injectBestLocation_2_0(
    const ::android::hardware::gnss::V2_0::GnssLocation& location) {
    ALOGV("%s not implemented", __func__);
    return false;
}

}  // namespace renesas
}  // namespace V2_0
}  // namespace gnss
}  // namespace hardware
}  // namespace android
