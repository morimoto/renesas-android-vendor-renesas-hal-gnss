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
#define LOG_TAG "GnssRenesasMeasurementProvider"
#include <log/log.h>

#include "include/MeasurementProvider.h"
#include "include/GnssMeasurementSync.h"

namespace android::hardware::gnss::V2_0::renesas {

MeasurementProvider::MeasurementProvider() :
    mBuilder(std::make_unique<MeasurementBuilder>()) {}

void MeasurementProvider::StartProviding() {
    mThreadExit = false;

    if (!mGnssMeasurementsCallbackThread.joinable()) {
        mGnssMeasurementsCallbackThread =
            std::thread(&MeasurementProvider::Provide, this);
    }
}

void MeasurementProvider::StopProviding() {
    mThreadExit = true;
    mCallbackCond.notify_one();

    if (mGnssMeasurementsCallbackThread.joinable()) {
        mGnssMeasurementsCallbackThread.join();
    }
}

void MeasurementProvider::Provide() {
    auto& syncInstance = GnssMeasurementSync::GetInstance();

    while (!mThreadExit) {
        auto data = new GnssData_2_0();
        auto error = mBuilder->Build(data);

        if (mEnabled && error == MBError::SUCCESS) {
            if (mGnssMeasurementsCbIface_1_1) {
                mGnssMeasurementsCbIface_1_1->gnssMeasurementCb(DataV2_0ToDataV1_1(*data));
            }
            if (mGnssMeasurementsCbIface_2_0) {
                mGnssMeasurementsCbIface_2_0->gnssMeasurementCb_2_0(*data);
            }
            syncInstance.NotifyEventOccured();
        }

        std::unique_lock<std::mutex> lock(mCallbackMutex);
        mCallbackCond.wait_for(lock, std::chrono::seconds(1));
    }
}

void MeasurementProvider::setMeasxCallback_1_1(IGnssMeasxCb_1_1 measxCb) {
    mGnssMeasurementsCbIface_1_1 = measxCb;
}

void MeasurementProvider::setMeasxCallback_2_0(IGnssMeasxCb_2_0 measxCb) {
    mGnssMeasurementsCbIface_2_0 = measxCb;
}

void MeasurementProvider::setEnabled(bool isEnabled) {
    mEnabled = isEnabled;
}

GnssData_1_1    MeasurementProvider::DataV2_0ToDataV1_1(const GnssData_2_0& v2_0) {
    auto v1_1 = new GnssData_1_1();
    v1_1->measurements.resize(v2_0.measurements.size());
    for (size_t i = 0; i < v2_0.measurements.size(); i++) {
        v1_1->measurements[i] = v2_0.measurements[i].v1_1;
    }
    v1_1->clock = v2_0.clock;
    return *v1_1;
}

}