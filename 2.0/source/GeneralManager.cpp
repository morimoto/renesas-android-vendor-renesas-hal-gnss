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
#define LOG_TAG "GnssRenesasHal"
#define LOG_NDEBUG 1

#include <android-base/logging.h>

#include "include/GeneralManager.h"

#include "include/DeviceScanner.h"
#include "include/LocationProvider.h"
#include "include/FakeLocationProvider.h"
#include "include/FakeReader.h"
#include "include/TtyReader.h"

namespace android::hardware::gnss::V2_0::renesas {

GeneralManager::GeneralManager() : mIsRun(false) {
    mDeviceScanner = std::make_unique<DeviceScanner>(this);
    mReceiverStatus = GnssReceiverStatus::WAIT_FOR_RECEIVER;
}

GeneralManager::~GeneralManager() {
    GnssStop();

    if (mReceiverInfoThread.joinable()) {
        mReceiverInfoThread.join();
    }

    if (mConfigThread.joinable()) {
        mConfigThread.join();
    }

    if (mLocationProvider) {
        mLocationProvider->StopProviding();
    }

    if (mSvInfoProvider) {
        mSvInfoProvider->StopProviding();
    }
}

void GeneralManager::StopToChange() {
    ALOGV("%s", __func__);

    if (nullptr != mReader) {
        mReceiverStatus = GnssReceiverStatus::WAIT_FOR_RECEIVER;
        mReader->Stop();
        mReceiver->GetTransport()->Reset();
        mReader = nullptr;
    }
}

void GeneralManager::StartAfterChange() {
    ALOGV("%s", __func__);
    mReceiver = mDeviceScanner->GetReceiver();

    if (nullptr == mReceiver) {
        ALOGW("No GNSS receiver");
        mReceiverStatus = GnssReceiverStatus::WAIT_FOR_RECEIVER;
    } else if (mIsRun) {
        mReceiverStatus = GnssReceiverStatus::RECEIVER_FOUND;
        std::shared_ptr<Transport> transport = mReceiver->GetTransport();
        transport->Reset();
        mReader = std::make_unique<TtyReader>(transport);
        mReader->Start();
        RunConfig();
    }
}

NmeaVersion GeneralManager::GetNmeaProtocol() {
    auto swVersion = mReceiver->GetSwVersion();

    switch (swVersion) {
    case SWVersion::SPG_201:
    case SWVersion::SPG_301:
        return NmeaVersion::NMEAv41;

    default:
        return NmeaVersion::NMEAv23;
    }
}

void GeneralManager::GnssCriticalError(std::string what) {
    GnssStop();
    CHECK_EQ(0, 1) << "Gnss HAL failed! Error message: " << what;
}

android::status_t GeneralManager::Run() {
    ALOGV("%s", __func__);

    if (auto err = mDeviceScanner->Scan(); err != DSError::Success) {
        ALOGW("No GNSS receiver");

        // might be connected later, check on gnssStart()
        if (err == DSError::NoReceiver) {
            mReceiverStatus = GnssReceiverStatus::WAIT_FOR_RECEIVER;
        } else {
            return ::android::UNKNOWN_ERROR;
        }
    }

    mUbxMsgHandler = std::make_unique<UbxMsgHandler>();
    mNmeaMsgHandler = std::make_unique<NmeaMsgHandler>();
    mUbxMsgHandler->StartProcessing();
    mNmeaMsgHandler->StartProcessing();
    mReceiver = mDeviceScanner->GetReceiver();
    if (mReceiver) {
        mReceiverStatus = GnssReceiverStatus::RECEIVER_FOUND;
        std::shared_ptr<Transport> transport = mReceiver->GetTransport();

        if (GnssReceiverType::FakeReceiver == mReceiver->GetReceiverType()) {
            SetupLocationProvider();
            mReceiverStatus = GnssReceiverStatus::READY;
            return ::android::OK;
        }

        mReader = std::make_unique<TtyReader>(transport);
        mReader->Start();
        if (!mConfigThread.joinable()) {
            mConfigThread = std::thread(&GeneralManager::RunConfig, this);
        }
    }

    SetupLocationProvider();
    SetupSvInfoProvider();
    SetupMeasurementProvider();
    mIsRun = true;
    return ::android::OK;
}

void GeneralManager::RunConfig() {
    ALOGV("%s", __func__);
    mReceiverStatus = GnssReceiverStatus::WAIT_FOR_CONFIG;
    Configurator config(mReceiver);

    if (CError::Success != config.Config()) {
        GnssCriticalError("Receiver config failed");
    }

    mNmeaMsgHandler->UpdateProtocolVersion(GetNmeaProtocol());
    SetupMeasurementProvider();
    mReceiverStatus = GnssReceiverStatus::CONFIG_DONE;
}

GMError GeneralManager::SetCallbackV1_0(const GnssCbPtr_1_0& cb) {
    if (GMError::SUCCESS != FillCallback(cb, mGnssCallback_1_0)) {
        return GMError::FAIL;
    }

    mLocationProvider->setCallback_1_0(mGnssCallback_1_0);

    if (!mReceiver || GnssReceiverType::FakeReceiver != mReceiver->GetReceiverType()) {
        mSvInfoProvider->setCallback_1_0(mGnssCallback_1_0);
    }

    return GMError::SUCCESS;
}

GMError GeneralManager::SetCallbackV1_1(const GnssCbPtr_1_1& cb) {
    if (GMError::SUCCESS != FillCallback(cb, mGnssCallback_1_1)) {
        return GMError::FAIL;
    }

    auto gnssName = "Renesas GNSS Implementation v1.1";
    mGnssCallback_1_1->gnssNameCb(gnssName);

    if (!mReceiver || GnssReceiverType::FakeReceiver != mReceiver->GetReceiverType()) {
        mSvInfoProvider->setCallback_1_1(mGnssCallback_1_1);
    }

    return GMError::SUCCESS;
}

GMError GeneralManager::SetCallbackV2_0(const GnssCbPtr_2_0& cb) {
    if (GMError::SUCCESS != FillCallback(cb, mGnssCallback_2_0)) {
        return GMError::FAIL;
    }

    auto gnssName = "Renesas GNSS Implementation v2.0";
    mGnssCallback_2_0->gnssNameCb(gnssName);

    mLocationProvider->setCallback_2_0(mGnssCallback_2_0);

    if (!mReceiver || GnssReceiverType::FakeReceiver != mReceiver->GetReceiverType()) {
        mSvInfoProvider->setCallback_2_0(mGnssCallback_2_0);
    }

    return GMError::SUCCESS;
}

GMError GeneralManager::CleanUpCb() {
    if (mLocationProvider) {
        mLocationProvider->SetEnabled(false);
    }

    if (mSvInfoProvider) {
        mSvInfoProvider->SetEnabled(false);
    }

    if (mGnssCallback_1_0) {
        mGnssCallback_1_0.clear();
    }

    if (mGnssCallback_1_1) {
        mGnssCallback_1_1.clear();
    }

    if (mGnssCallback_2_0) {
        mGnssCallback_2_0.clear();
    }

    return GMError::SUCCESS;
}

void GeneralManager::setUpdatePeriod(uint32_t updateIntervalMs) {
    mUpdateIntervalUs = updateIntervalMs * 1000;

    if (mLocationProvider) {
        mLocationProvider->SetUpdateInterval(mUpdateIntervalUs);
    }

    if (mSvInfoProvider) {
        mSvInfoProvider->SetUpdateInterval(mUpdateIntervalUs);
    }
}

GMError GeneralManager::GnssStart() {
    ALOGV("%s", __func__);

    if (mReceiverStatus == GnssReceiverStatus::WAIT_FOR_RECEIVER) {
        ALOGE("%s: Gnss receiver not ready", __func__);
        return GMError::FAIL;
    }

    if (!mLocationProvider) {
        ALOGE("No location provider, can't start Gnss HAL");
        return GMError::FAIL;
    }

    mLocationProvider->SetEnabled(true);

    if (GnssReceiverType::FakeReceiver != mReceiver->GetReceiverType()) {
        mSvInfoProvider->SetEnabled(true);
    }

    if (mGnssCallback_1_0) {
        mGnssCallback_1_0->gnssStatusCb(IGnssCallback::GnssStatusValue::SESSION_BEGIN);
    }
    if (mGnssCallback_1_1) {
        mGnssCallback_1_1->gnssStatusCb(IGnssCallback::GnssStatusValue::SESSION_BEGIN);
    }
    if (mGnssCallback_2_0) {
        mGnssCallback_2_0->gnssStatusCb(IGnssCallback::GnssStatusValue::SESSION_BEGIN);
    }

    return GMError::SUCCESS;
}

GMError GeneralManager::GnssStop() {
    ALOGV("%s", __func__);

    if (mLocationProvider) {
        mLocationProvider->SetEnabled(false);
    }
    if (mSvInfoProvider) {
        mSvInfoProvider->SetEnabled(false);
    }
    if (mGnssCallback_1_0) {
        mGnssCallback_1_0->gnssStatusCb(IGnssCallback::GnssStatusValue::SESSION_END);
    }
    if (mGnssCallback_1_1) {
        mGnssCallback_1_1->gnssStatusCb(IGnssCallback::GnssStatusValue::SESSION_END);
    }
    if (mGnssCallback_2_0) {
        mGnssCallback_2_0->gnssStatusCb(IGnssCallback::GnssStatusValue::SESSION_END);
    }
    return GMError::SUCCESS;
}

GMError GeneralManager::SetupLocationProvider() {
    if (mReceiver && GnssReceiverType::FakeReceiver == mReceiver->GetReceiverType()) {
        mLocationProvider = std::make_unique<FakeLocationProvider>
                                (mUpdateIntervalUs);
    } else {
        mLocationProvider = std::make_unique<LocationProvider>
                                (mUpdateIntervalUs);
    }

    mLocationProvider->StartProviding();

    return GMError::SUCCESS;
}

GMError GeneralManager::SetupMeasurementProvider() {
    mGnssMeasurement = new GnssMeasurement();
    return GMError::SUCCESS;
}


GMError GeneralManager::SetupSvInfoProvider() {
    mSvInfoProvider =
        std::make_unique<GnssInfoProvider>(mUpdateIntervalUs);
    mSvInfoProvider->StartProviding();

    return GMError::SUCCESS;
}

sp<IGnssMeasurement> GeneralManager::getExtensionGnssMeasurement_v2_0() {
    return mGnssMeasurement;
}

bool GeneralManager::IsRun() {
    return mIsRun;
}

} // namespace android::hardware::gnss::V2_0::renesas
