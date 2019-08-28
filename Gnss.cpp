/*
 * Copyright (C) 2017 The Android Open Source Project
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

#define LOG_TAG "GnssRenesasHAL"
#define LOG_NDEBUG 1

#include <log/log.h>
#include <cutils/properties.h>

#include "Gnss.h"

namespace android {
namespace hardware {
namespace gnss {
namespace V1_0 {
namespace renesas {

sp<IGnssCallback> Gnss::sGnssCbIface = nullptr;
bool Gnss::sWakelockHeldGnss = false;
bool Gnss::sWakelockHeldFused = false;

Gnss::Gnss(void) :
    mDeathRecipient(new GnssHidlDeathRecipient(this))
{
    char mode[PROPERTY_VALUE_MAX];
    if (property_get("ro.boot.gps.mode", mode, "tty") > 0) {
        if (strcmp(mode, "fake") == 0) {
            ALOGI("Using FAKE backend");
            mGnssHwIface = new GnssHwFAKE();
            mGnssHwIface->SetUpHandleThread();
        }
    }

    if (mGnssHwIface == nullptr) {
        mGnssHwIface = new GnssHwTTY(-1);
        mGnssHwIface->SetUpHandleThread();
        mGnssMeasurement = new GnssMeasurementImpl();
        ALOGI("Using TTY HW backend");
    }
}

Gnss::~Gnss(void)
{
}

void Gnss::acquireWakelockGnss() {
    sWakelockHeldGnss = true;
    updateWakelock();
}

void Gnss::releaseWakelockGnss() {
    sWakelockHeldGnss = false;
    updateWakelock();
}

void Gnss::acquireWakelockFused() {
    sWakelockHeldFused = true;
    updateWakelock();
}

void Gnss::releaseWakelockFused() {
    sWakelockHeldFused = false;
    updateWakelock();
}

void Gnss::updateWakelock() {
    // Track the state of the last request - in case the wake lock in the layer above is reference
    // counted.
    static bool sWakelockHeld = false;

    if (sGnssCbIface == nullptr) {
        ALOGE("%s: GNSS Callback Interface configured incorrectly", __func__);
        return;
    }

    if (sWakelockHeldGnss || sWakelockHeldFused) {
        if (!sWakelockHeld) {
            ALOGI("%s: GNSS HAL Wakelock acquired due to gps: %d, fused: %d", __func__,
                    sWakelockHeldGnss, sWakelockHeldFused);
            sWakelockHeld = true;
            auto ret = sGnssCbIface->gnssAcquireWakelockCb();
            if (!ret.isOk()) {
                ALOGE("%s: Unable to invoke callback", __func__);
            }
        }
    } else {
        if (sWakelockHeld) {
            ALOGI("%s: GNSS HAL Wakelock released", __func__);
        } else  {
            // To avoid burning power, always release, even if logic got here with sWakelock false
            // which it shouldn't, unless underlying *.h implementation makes duplicate requests.
            ALOGW("%s: GNSS HAL Wakelock released, duplicate request", __func__);
        }
        sWakelockHeld = false;
        auto ret = sGnssCbIface->gnssReleaseWakelockCb();
        if (!ret.isOk()) {
            ALOGE("%s: Unable to invoke callback", __func__);
        }
    }
}

Return<sp<IAGnssRil>> Gnss::getExtensionAGnssRil()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssRil == nullptr) {
        ALOGE("%s GnssRil interface not implemented by GNSS HAL", __func__);
    }
    return mGnssRil;
}

Return<sp<IGnssConfiguration>> Gnss::getExtensionGnssConfiguration()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssConfig == nullptr) {
        const GnssConfigurationInterface* gnssConfigIface =
                static_cast<const GnssConfigurationInterface*>(
                        mGnssIface->get_extension(GNSS_CONFIGURATION_INTERFACE));

        if (gnssConfigIface == nullptr) {
            ALOGE("%s GnssConfiguration interface not implemented by GNSS HAL", __func__);
        } else {
            mGnssConfig = new GnssConfiguration();
        }
    }
    return mGnssConfig;
}

Return<sp<IGnssGeofencing>> Gnss::getExtensionGnssGeofencing()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssGeofencingIface == nullptr) {
        const GpsGeofencingInterface* gpsGeofencingIface =
                static_cast<const GpsGeofencingInterface*>(
                        mGnssIface->get_extension(GPS_GEOFENCING_INTERFACE));

        if (gpsGeofencingIface == nullptr) {
            ALOGE("%s GnssGeofencing interface not implemented by GNSS HAL", __func__);
        } else {
            mGnssGeofencingIface = new GnssGeofencing();
        }
    }

    return mGnssGeofencingIface;
}

Return<sp<IAGnss>> Gnss::getExtensionAGnss()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mAGnssIface == nullptr) {
        const AGpsInterface* agpsIface = static_cast<const AGpsInterface*>(
                mGnssIface->get_extension(AGPS_INTERFACE));
        if (agpsIface == nullptr) {
            ALOGE("%s AGnss interface not implemented by GNSS HAL", __func__);
        } else {
            mAGnssIface = new AGnss(agpsIface);
        }
    }
    return mAGnssIface;
}

Return<sp<IGnssNi>> Gnss::getExtensionGnssNi()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssNi == nullptr) {
        const GpsNiInterface* gpsNiIface = static_cast<const GpsNiInterface*>(
                mGnssIface->get_extension(GPS_NI_INTERFACE));
        if (gpsNiIface == nullptr) {
            ALOGE("%s GnssNi interface not implemented by GNSS HAL", __func__);
        } else {
            mGnssNi = new GnssNi();
        }
    }
    return mGnssNi;
}

Return<sp<IGnssMeasurement>> Gnss::getExtensionGnssMeasurement() {
    if (mGnssHwIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssMeasurement == nullptr) {
        ALOGE("%s GnssMeasurement interface not implemented by GNSS HAL", __func__);
    }
    return mGnssMeasurement;
}

Return<sp<IGnssNavigationMessage>> Gnss::getExtensionGnssNavigationMessage() {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssNavigationMessage == nullptr) {
        const GpsNavigationMessageInterface* gpsNavigationMessageIface =
                static_cast<const GpsNavigationMessageInterface*>(
                        mGnssIface->get_extension(GPS_NAVIGATION_MESSAGE_INTERFACE));

        if (gpsNavigationMessageIface == nullptr) {
            ALOGE("%s GnssNavigationMessage interface not implemented by GNSS HAL",
                  __func__);
        } else {
            mGnssNavigationMessage = new GnssNavigationMessage();
        }
    }

    return mGnssNavigationMessage;
}

Return<sp<IGnssXtra>> Gnss::getExtensionXtra()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssXtraIface == nullptr) {
        const GpsXtraInterface* gpsXtraIface = static_cast<const GpsXtraInterface*>(
                mGnssIface->get_extension(GPS_XTRA_INTERFACE));

        if (gpsXtraIface == nullptr) {
            ALOGE("%s GnssXtra interface not implemented by HAL", __func__);
        } else {
            mGnssXtraIface = new GnssXtra();
        }
    }

    return mGnssXtraIface;
}

Return<sp<IGnssDebug>> Gnss::getExtensionGnssDebug()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssDebug == nullptr) {
        const GpsDebugInterface* gpsDebugIface = static_cast<const GpsDebugInterface*>(
                mGnssIface->get_extension(GPS_DEBUG_INTERFACE));

        if (gpsDebugIface == nullptr) {
            ALOGE("%s: GnssDebug interface is not implemented by HAL", __func__);
        } else {
            mGnssDebug = new GnssDebug();
        }
    }

    return mGnssDebug;
}

Return<sp<IGnssBatching>> Gnss::getExtensionGnssBatching()  {
    if (mGnssIface == nullptr) {
        ALOGE("%s: Gnss interface is unavailable", __func__);
        return nullptr;
    }

    if (mGnssBatching == nullptr) {
        hw_module_t* module;
        const FlpLocationInterface* flpLocationIface = nullptr;
        int err = hw_get_module(FUSED_LOCATION_HARDWARE_MODULE_ID, (hw_module_t const**)&module);

        if (err != 0) {
            ALOGE("gnss flp hw_get_module failed: %d", err);
        } else if (module == nullptr) {
            ALOGE("Fused Location hw_get_module returned null module");
        } else if (module->methods == nullptr) {
            ALOGE("Fused Location hw_get_module returned null methods");
        } else {
            hw_device_t* device;
            err = module->methods->open(module, FUSED_LOCATION_HARDWARE_MODULE_ID, &device);
            if (err != 0) {
                ALOGE("flpDevice open failed: %d", err);
            } else {
                flp_device_t * flpDevice = reinterpret_cast<flp_device_t*>(device);
                flpLocationIface = flpDevice->get_flp_interface(flpDevice);
            }
        }

        if (flpLocationIface == nullptr) {
            ALOGE("%s: GnssBatching interface is not implemented by HAL", __func__);
        } else {
            mGnssBatching = new GnssBatching(flpLocationIface);
        }
    }
    return mGnssBatching;
}

Return<bool> Gnss::setCallback(const sp<IGnssCallback>& callback)
{
    if (callback == nullptr)  {
        ALOGE("%s: Null callback ignored", __func__);
        return false;
    }

    if (mGnssCbIface != NULL) {
        ALOGW("%s called more than once. Unexpected unless test.", __func__);
        mGnssCbIface->unlinkToDeath(mDeathRecipient);
    }

    mGnssCbIface = callback;
    mGnssCbIface->gnssSetCapabilitesCb(IGnssCallback::Capabilities::GEOFENCING |
                                       IGnssCallback::Capabilities::NAV_MESSAGES |
                                       IGnssCallback::Capabilities::MEASUREMENTS);

    mGnssCbIface->gnssSetSystemInfoCb({mGnssHwIface->GetYearOfHardware()});
    mGnssCbIface->gnssStatusCb(IGnssCallback::GnssStatusValue::NONE);

    mGnssHwIface->setCallback(callback);

    callback->linkToDeath(mDeathRecipient, 0 /*cookie*/);
    return true;
}

void Gnss::handleHidlDeath(void)
{
    ALOGW("GNSS service noticed HIDL death. Stopping all GNSS operations.");
}

Return<bool> Gnss::start(void)
{
    ALOGV("%s", __func__);
    return mGnssHwIface->start();
}

Return<bool> Gnss::stop(void)
{
    ALOGV("%s", __func__);
    return mGnssHwIface->stop();
}

Return<void> Gnss::cleanup(void)
{
    ALOGV("%s", __func__);

    mGnssHwIface->stop();
    return Void();
}

Return<bool> Gnss::injectLocation(double latitudeDegrees,
                                  double longitudeDegrees,
                                  float accuracyMeters)
{
    ALOGD("%s: latitudeDegrees=%f, longitudeDegrees=%f, accuracyMeters=%f",
        __func__, latitudeDegrees, longitudeDegrees, accuracyMeters);
    return true;
}

Return<bool> Gnss::injectTime(int64_t timeMs, int64_t timeReferenceMs,
                              int32_t uncertaintyMs)
{
    ALOGD("%s: timeMs=%zu, timeReferenceMs=%zu, uncertaintyMs=%d",
        __func__, timeMs, timeReferenceMs, uncertaintyMs);
    return true;
}

Return<void> Gnss::deleteAidingData(IGnss::GnssAidingData aidingDataFlags)
{
    ALOGD("%s: aidingDataFlags=0x%hx", __func__, aidingDataFlags);
    return Void();
}

Return<bool> Gnss::setPositionMode(IGnss::GnssPositionMode mode,
                                   IGnss::GnssPositionRecurrence recurrence,
                                   uint32_t minIntervalMs,
                                   uint32_t preferredAccuracyMeters,
                                   uint32_t preferredTimeMs)
{
    ALOGD("%s: mode=%d, recurrence=%d, minIntervalMs=%d, preferredAccuracyMeters=%d, preferredTimeMs=%d",
        __func__, (int)mode, recurrence, minIntervalMs, preferredAccuracyMeters, preferredTimeMs);
    mGnssHwIface->setUpdatePeriod(minIntervalMs);
    return true;
}

}  // namespace renesas
}  // namespace V1_0
}  // namespace gnss
}  // namespace hardware
}  // namespace android
