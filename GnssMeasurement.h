/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef android_hardware_gnss_V1_0_GnssMeasurement_H_
#define android_hardware_gnss_V1_0_GnssMeasurement_H_

#include <ThreadCreationWrapper.h>
#include <android/hardware/gnss/1.0/IGnssMeasurement.h>
#include <hidl/Status.h>
#include <thread>
#include <utils/SystemClock.h>

namespace android {
namespace hardware {
namespace gnss {
namespace V1_0 {
namespace renesas {

using ::android::hardware::gnss::V1_0::IGnssMeasurement;
using ::android::hardware::gnss::V1_0::IGnssMeasurementCallback;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::sp;

/*
 * Extended interface for GNSS Measurements support. Also contains wrapper methods to allow methods
 * from IGnssMeasurementCallback interface to be passed into the conventional implementation of the
 * GNSS HAL.
 */
struct GnssMeasurement : public IGnssMeasurement {
    GnssMeasurement();

    /*
     * Methods from ::android::hardware::gnss::V1_0::IGnssMeasurement follow.
     * These declarations were generated from IGnssMeasurement.hal.
     */
    Return<GnssMeasurementStatus> setCallback(
        const sp<IGnssMeasurementCallback>& callback) override;
    Return<void> close() override;

    void callbackThread(void);
private:
    std::atomic<bool> mThreadExit;
    static sp<IGnssMeasurementCallback> sGnssMeasurementsCbIface;
    std::thread mGnssMeasurementsCallbackThread;
    std::mutex mCallbackMutex;
    std::condition_variable mCallbackCond;
};

}  // namespace renesas
}  // namespace V1_0
}  // namespace gnss
}  // namespace hardware
}  // namespace android

#endif  // android_hardware_gnss_V1_0_GnssMeasurement_H_
