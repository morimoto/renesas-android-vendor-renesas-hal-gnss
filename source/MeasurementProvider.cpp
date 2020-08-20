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
#define LOG_TAG "GnssRenesasMeasurementProvider"

#include "include/MeasurementProvider.h"

#include <log/log.h>

#include "include/GnssMeasurementSync.h"

namespace android::hardware::gnss::V2_1::renesas {

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
    ALOGV("%s", __func__);
    auto& syncInstance = GnssMeasurementSync::GetInstance();

    while (!mThreadExit) {
        GnssData_2_0 data = {};
        auto error = mBuilder->Build(data);

        if (mEnabled && error == MBError::SUCCESS) {
            if (mGnssMeasurementsCbIface_1_0) {
                mGnssMeasurementsCbIface_1_0->GnssMeasurementCb(DataV2_0ToDataV1_0(data));
            }
            if (mGnssMeasurementsCbIface_1_1) {
                mGnssMeasurementsCbIface_1_1->gnssMeasurementCb(DataV2_0ToDataV1_1(data));
            }
            if (mGnssMeasurementsCbIface_2_0) {
                mGnssMeasurementsCbIface_2_0->gnssMeasurementCb_2_0(data);
            }
            syncInstance.NotifyEventOccured();
        }

        std::unique_lock<std::mutex> lock(mCallbackMutex);
        mCallbackCond.wait_for(lock, std::chrono::seconds(1));
    }
}

void MeasurementProvider::setMeasxCallback_1_0(IGnssMeasxCb_1_0 measxCb) {
    mGnssMeasurementsCbIface_1_0 = measxCb;
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

GnssData_1_0    MeasurementProvider::DataV2_0ToDataV1_0(const GnssData_2_0& data_2_0) {
    GnssData_1_0 data_1_0;
    data_1_0.measurementCount = data_2_0.measurements.size();
    for (size_t i = 0; i < data_2_0.measurements.size(); i++) {
        data_1_0.measurements[i] = data_2_0.measurements[i].v1_1.v1_0;
    }
    data_1_0.clock = data_2_0.clock;
    return data_1_0;
}

GnssData_1_1    MeasurementProvider::DataV2_0ToDataV1_1(const GnssData_2_0& data_2_0) {
    GnssData_1_1 data_1_1;
    data_1_1.measurements.resize(data_2_0.measurements.size());
    for (size_t i = 0; i < data_2_0.measurements.size(); i++) {
        data_1_1.measurements[i] = data_2_0.measurements[i].v1_1;
    }
    data_1_1.clock = data_2_0.clock;
    return data_1_1;
}

}
