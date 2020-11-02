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

#include <MeasurementProvider.h>

#include <log/log.h>

#include <GnssMeasurementSync.h>

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
        GnssData_2_1 data_2_1 = {};
        auto error = mBuilder->Build(data_2_1);

        if (mEnabled && error == MBError::SUCCESS) {
            for (auto provider : mProviders) {
                provider.second(data_2_1);
            }
            syncInstance.NotifyEventOccured();
        }

        std::unique_lock<std::mutex> lock(mCallbackMutex);
        mCallbackCond.wait_for(lock, std::chrono::seconds(1));
    }
}

void MeasurementProvider::setMeasxCallback_1_0(IGnssMeasxCb_1_0 measxCb) {
    static const std::string providerName = {"GnssMeasxCb_1_0"};

    mGnssMeasurementsCbIface_1_0 = measxCb;
    mProviders[providerName] = [this](const GnssData_2_1& data_2_1) {
        if (mGnssMeasurementsCbIface_1_0) {
            mGnssMeasurementsCbIface_1_0->GnssMeasurementCb(DataV2_1ToDataV1_0(data_2_1));
        }
    };
}

void MeasurementProvider::setMeasxCallback_1_1(IGnssMeasxCb_1_1 measxCb) {
    static const std::string providerName = {"GnssMeasxCb_1_1"};

    mGnssMeasurementsCbIface_1_1 = measxCb;
    mProviders[providerName] = [this](const GnssData_2_1& data_2_1) {
        if (mGnssMeasurementsCbIface_1_1) {
            mGnssMeasurementsCbIface_1_1->gnssMeasurementCb(DataV2_1ToDataV1_1(data_2_1));
        }
    };
}

void MeasurementProvider::setMeasxCallback_2_0(IGnssMeasxCb_2_0 measxCb) {
    static const std::string providerName = {"GnssMeasxCb_2_0"};

    mGnssMeasurementsCbIface_2_0 = measxCb;
    mProviders[providerName] = [this](const GnssData_2_1& data_2_1) {
        if (mGnssMeasurementsCbIface_2_0) {
            mGnssMeasurementsCbIface_2_0->gnssMeasurementCb_2_0(DataV2_1ToDataV2_0(data_2_1));
        }
    };
}

void MeasurementProvider::setMeasxCallback_2_1(IGnssMeasxCb_2_1 measxCb)
{
    static const std::string providerName = {"GnssMeasxCb_2_1"};

    mGnssMeasurementsCbIface_2_1 = measxCb;
    mProviders[providerName] = [this](const GnssData_2_1& data_2_1) {
        if (mGnssMeasurementsCbIface_2_1) {
            mGnssMeasurementsCbIface_2_1->gnssMeasurementCb_2_1(data_2_1);
        }
    };
}

void MeasurementProvider::setEnabled(bool isEnabled) {
    mEnabled = isEnabled;
}

GnssData_1_0 MeasurementProvider::DataV2_1ToDataV1_0(const GnssData_2_1& data_2_1) {
    GnssData_1_0 data_1_0;
    GnssData_2_0 data_2_0 = {DataV2_1ToDataV2_0(data_2_1)};
    data_1_0.measurementCount = data_2_0.measurements.size();
    for (size_t i = 0; i < data_2_0.measurements.size(); i++) {
        data_1_0.measurements[i] = data_2_0.measurements[i].v1_1.v1_0;
    }
    data_1_0.clock = data_2_0.clock;
    return data_1_0;
}

GnssData_1_1 MeasurementProvider::DataV2_1ToDataV1_1(const GnssData_2_1& data_2_1) {
    GnssData_1_1 data_1_1;
    GnssData_2_0 data_2_0 = {DataV2_1ToDataV2_0(data_2_1)};
    data_1_1.measurements.resize(data_2_0.measurements.size());
    for (size_t i = 0; i < data_2_0.measurements.size(); i++) {
        data_1_1.measurements[i] = data_2_0.measurements[i].v1_1;
    }
    data_1_1.clock = data_2_0.clock;
    return data_1_1;
}

GnssData_2_0 MeasurementProvider::DataV2_1ToDataV2_0(const GnssData_2_1 &data_2_1)
{
    GnssData_2_0 data_2_0;
    data_2_0.measurements.resize(data_2_1.measurements.size());
    for (size_t i = 0; i < data_2_1.measurements.size(); i++) {
        data_2_0.measurements[i] = data_2_1.measurements[i].v2_0;
    }
    data_2_0.clock=data_2_1.clock.v1_0;
    data_2_0.elapsedRealtime = data_2_1.elapsedRealtime;
    return data_2_0;
}

}
