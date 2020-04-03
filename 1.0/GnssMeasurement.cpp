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

#define LOG_TAG "GnssRenesasHAL_GnssMeasurementInterface"

#include <memory>

#include "GnssMeasToLocSync.h"
#include "GnssMeasurement.h"
#include "GnssMeasQueue.h"
#include "GnssIParser.h"

namespace android {
namespace hardware {
namespace gnss {
namespace V1_0 {
namespace renesas {

static const bool on = true;
static const bool off = false;
static const int8_t measBeforeLocationEventsNum = 2; // value depends on CTS testGnssMeasurementWhenNoLocation

sp<MeasurementCb> GnssMeasurementImpl::sGnssMeasurementsCbIface = nullptr;

GnssMeasurementImpl::GnssMeasurementImpl():
    mThreadExit(false)
{}

// Methods from ::android::hardware::gnss::V1_0::IGnssMeasurement follow.
Return<GnssMeasurementImpl::GnssMeasurementStatus> GnssMeasurementImpl::setCallback(
        const sp<IGnssMeasurementCallback>& callback)
{

    if (sGnssMeasurementsCbIface != nullptr) {
        ALOGE("%s: GnssMeasurements already init", __func__);
        return GnssMeasurementStatus::ERROR_ALREADY_INIT;
    }

    GnssMeasToLocSync& syncInstance = GnssMeasToLocSync::getInstance();
    syncInstance.SetEventsToWait(measBeforeLocationEventsNum);

    sGnssMeasurementsCbIface = callback;
    ALOGD("%s: GnssMeasurements initialized", __func__);

    mThreadExit = false;
    if (!mGnssMeasurementsCallbackThread.joinable()) {
        mGnssMeasurementsCallbackThread = std::thread(&GnssMeasurementImpl::callbackThread, this);
    }

    return GnssMeasurementStatus::SUCCESS;
}

Return<void> GnssMeasurementImpl::close()
{
    if (sGnssMeasurementsCbIface == nullptr) {
        ALOGD("%s: called before setCallback", __func__);
        return Void();
    }

    GnssMeasToLocSync& syncInstance = GnssMeasToLocSync::getInstance();
    syncInstance.SetEventsToWait(0);

    sGnssMeasurementsCbIface = nullptr;
    mThreadExit = true;
    mCallbackCond.notify_one();
    if (mGnssMeasurementsCallbackThread.joinable()) {
        mGnssMeasurementsCallbackThread.join();
    }
    ALOGD("%s: GnssMeasurements closed", __func__);

    return Void();
}

void GnssMeasurementImpl::callbackThread(void)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    GnssMeasToLocSync& syncInstance = GnssMeasToLocSync::getInstance();
    GnssMeasQueue& instance = GnssMeasQueue::getInstance();
    instance.setState(on);
    while (!mThreadExit) {
        uint8_t flagReady = 0;
        auto data = std::make_unique<MeasurementCb::GnssData> ();
        while (!instance.empty()) {
            auto parser = instance.pop();
            if (nullptr == parser) {
                ALOGV("[%s, line %d] parser is null", __func__, __LINE__);
                continue;
            }
            flagReady |= parser->retrieveSvInfo(*data);

            if (sGnssMeasurementsCbIface != nullptr && GnssIParser::Ready == flagReady) {
                sGnssMeasurementsCbIface->GnssMeasurementCb(*data);
                ALOGD("GNSS Measurements sent");
                syncInstance.UpdateStatus((int8_t)-1);
                break;
            }
        }
        // TODO: figure out what is this for - maybe
        // in 2.0 we should do it if (!enableFullTracking)
        std::unique_lock<std::mutex> lock(mCallbackMutex);
        mCallbackCond.wait_for(lock,
                      std::chrono::seconds(1));
    }

    instance.setState(off);
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
}

}  // namespace renesas
}  // namespace V1_0
}  // namespace gnss
}  // namespace hardware
}  // namespace android
