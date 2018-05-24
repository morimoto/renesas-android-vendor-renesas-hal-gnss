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

#ifndef ANDROID_HARDWARE_GNSS_V1_0_KINGFISHER_H_
#define ANDROID_HARDWARE_GNSS_V1_0_KINGFISHER_H_

#include <android/hardware/gnss/1.0/IGnss.h>
#include <hidl/Status.h>

#include <AGnss.h>
#include <AGnssRil.h>
#include <GnssBatching.h>
#include <GnssConfiguration.h>
#include <GnssDebug.h>
#include <GnssGeofencing.h>
#include <GnssMeasurement.h>
#include <GnssNavigationMessage.h>
#include <GnssNi.h>
#include <GnssXtra.h>

#include "GnssHw.h"

namespace android {
namespace hardware {
namespace gnss {
namespace V1_0 {
namespace salvator {

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::sp;

struct Gnss : public IGnss {

    Gnss(void);
    ~Gnss(void);

    /*
     * Methods from ::android::hardware::gnss::V1_0::IGnss follow.
     */
    Return<bool> setCallback(const sp<IGnssCallback>& callback) override;
    Return<bool> start(void) override;
    Return<bool> stop(void) override;
    Return<void> cleanup(void) override;

    Return<bool> injectLocation(double latitudeDegrees,
                                double longitudeDegrees,
                                float accuracyMeters) override;

    Return<bool> injectTime(int64_t timeMs,
                            int64_t timeReferenceMs,
                            int32_t uncertaintyMs) override;

    Return<void> deleteAidingData(IGnss::GnssAidingData aidingDataFlags) override;

    Return<bool> setPositionMode(IGnss::GnssPositionMode mode,
                                 IGnss::GnssPositionRecurrence recurrence,
                                 uint32_t minIntervalMs,
                                 uint32_t preferredAccuracyMeters,
                                 uint32_t preferredTimeMs) override;

    Return<sp<IAGnssRil>> getExtensionAGnssRil(void) override;
    Return<sp<IGnssGeofencing>> getExtensionGnssGeofencing(void) override;
    Return<sp<IAGnss>> getExtensionAGnss(void) override;
    Return<sp<IGnssNi>> getExtensionGnssNi(void) override;
    Return<sp<IGnssMeasurement>> getExtensionGnssMeasurement(void) override;
    Return<sp<IGnssNavigationMessage>> getExtensionGnssNavigationMessage(void) override;
    Return<sp<IGnssXtra>> getExtensionXtra(void) override;
    Return<sp<IGnssConfiguration>> getExtensionGnssConfiguration(void) override;
    Return<sp<IGnssDebug>> getExtensionGnssDebug(void) override;
    Return<sp<IGnssBatching>> getExtensionGnssBatching(void) override;

    /*
     * Wakelock consolidation, only needed for dual use of a gps.h & fused_location.h HAL
     *
     * Ensures that if the last call from either legacy .h was to acquire a wakelock, that a
     * wakelock is held.  Otherwise releases it.
     */
    static void acquireWakelockFused();
    static void releaseWakelockFused();

private:
    /*
     * For handling system-server death while GNSS service lives on.
     */
    class GnssHidlDeathRecipient : public hidl_death_recipient {
      public:
        GnssHidlDeathRecipient(const sp<Gnss> gnss) : mGnss(gnss) { }

        virtual void serviceDied(uint64_t /*cookie*/,
                const wp<::android::hidl::base::V1_0::IBase>& /*who*/) {
            mGnss->handleHidlDeath();
        }
      private:
        sp<Gnss> mGnss;
    };

    // for wakelock consolidation, see above
    static void acquireWakelockGnss();
    static void releaseWakelockGnss();
    static void updateWakelock();
    static bool sWakelockHeldGnss;
    static bool sWakelockHeldFused;

    /* Cleanup for death notification */
    void handleHidlDeath(void);

    sp<GnssXtra> mGnssXtraIface = nullptr;
    sp<AGnssRil> mGnssRil = nullptr;
    sp<GnssGeofencing> mGnssGeofencingIface = nullptr;
    sp<AGnss> mAGnssIface = nullptr;
    sp<GnssNi> mGnssNi = nullptr;
    sp<GnssMeasurement> mGnssMeasurement = nullptr;
    sp<GnssNavigationMessage> mGnssNavigationMessage = nullptr;
    sp<GnssDebug> mGnssDebug = nullptr;
    sp<GnssConfiguration> mGnssConfig = nullptr;
    sp<GnssBatching> mGnssBatching = nullptr;

    sp<GnssHwIface>             mGnssHwIface = nullptr;
    sp<GnssHidlDeathRecipient>  mDeathRecipient = nullptr;
    sp<IGnssCallback>           mGnssCbIface = nullptr;

    const GpsInterface* mGnssIface = nullptr;
    static sp<IGnssCallback> sGnssCbIface;
};

}  // namespace salvator
}  // namespace V1_0
}  // namespace gnss
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_GNSS_V1_0_KINGFISHER_H_
