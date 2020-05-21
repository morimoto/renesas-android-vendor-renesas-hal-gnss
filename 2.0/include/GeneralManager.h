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
#pragma once

#include "include/ILocationProvider.h"
#include "include/GnssInfoProvider.h"
#include "include/UbloxMsgHandler.h"
#include "include/NmeaMsgHandler.h"
#include "include/IGnssReceiver.h"
#include "include/DeviceScanner.h"
#include "include/Configurator.h"
#include "include/IReader.h"
#include "include/GnssMeasurement.h"

namespace android::hardware::gnss::V2_0::renesas {

enum class GMError : uint8_t {
    SUCCESS,
    INTERNAL,
    NO_RECEIVER,
    FAIL,
};

enum class GnssReceiverStatus : uint8_t {
    WAIT_FOR_RECEIVER,
    RECEIVER_FOUND,
    WAIT_FOR_SETUP,
    SETUP_DONE,
    WAIT_FOR_CONFIG,
    CONFIG_DONE,
    READY = CONFIG_DONE,
};

class DeviceScanner;
class GnssImpl;

class GeneralManager {
public:
    using GnssCbPtr_1_1 = android::sp<android::hardware::gnss::V1_1::IGnssCallback>;
    using GnssCbPtr_2_0 = android::sp<android::hardware::gnss::V2_0::IGnssCallback>;
    using GnssHalImpl = android::hardware::gnss::V2_0::renesas::GnssImpl;
    using IGnssMeasurement_1_1 = android::hardware::gnss::V2_0::IGnssMeasurement;
    using IGnssMeasurement_2_0 = android::hardware::gnss::V2_0::IGnssMeasurement;
    /*!
     * \brief GeneralManager
     * \param gnssHal
     */
    GeneralManager();
    ~GeneralManager();

    /*!
     * \brief Run
     * \return
     */
    android::status_t Run();

    /*!
     * \brief SetCallbackV1_1
     * \param cb
     * \return
     */
    GMError SetCallbackV1_1(const GnssCbPtr_1_1& cb);

    /*!
     * \brief SetCallbackV2_0
     * \param cb
     * \return
     */
    GMError SetCallbackV2_0(const GnssCbPtr_2_0& cb);

    /*!
     * \brief CleanUpCb
     * \return
     */
    GMError CleanUpCb();

    void setUpdatePeriod(uint32_t updateIntervalMs);

    sp<IGnssMeasurement> getExtensionGnssMeasurement_v2_0();

    /*!
     * \brief GnssStart
     * \return
     */
    GMError GnssStart();

    /*!
     * \brief GnssStop
     * \return
     */
    GMError GnssStop();

    /*!
     * \brief StopToChange
     * \return
     */
    void StopToChange();

    /*!
     * \brief StartAfterChange
     * \return
     */
    void StartAfterChange();

protected:
    /*!
     * \brief SetupLocationProvider
     * \return
     */
    GMError SetupLocationProvider();

    /*!
     * \brief SetupMeasurementProvider
     * \return
     */
    GMError SetupMeasurementProvider();

    GMError SetupSvInfoProvider();

private:
    GeneralManager(const GeneralManager&) = delete;
    GeneralManager operator=(const GeneralManager&) = delete;

    NmeaVersion GetNmeaProtocol();
    void GnssCriticalError(std::string what);
    void UpdateReceiverInfo();
    void RunConfig();

    GnssCbPtr_1_1 mGnssCallback_1_1;
    GnssCbPtr_2_0 mGnssCallback_2_0;
    std::shared_ptr<android::hardware::gnss::V2_0::renesas::GnssImpl> mGnssImpl;

    std::unique_ptr<DeviceScanner> mDeviceScanner;
    std::shared_ptr<IGnssReceiver> mReceiver;
    std::unique_ptr<IReader> mReader;
    std::unique_ptr<ILocationProvider> mLocationProvider;
    std::unique_ptr<GnssInfoProvider> mSvInfoProvider;
    sp<GnssMeasurement> mGnssMeasurement;
    std::unique_ptr<UbxMsgHandler> mUbxMsgHandler;
    std::unique_ptr<NmeaMsgHandler> mNmeaMsgHandler;

    uint32_t mUpdateIntervalUs = 1000 * 1000; // set default interval to 1 sec

    GnssReceiverStatus mReceiverStatus;
    std::thread mReceiverInfoThread;
    std::thread mConfigThread;
    std::condition_variable mReceiverCv;
    std::mutex mLock;
    bool mIsRun;
};

} // android::hardware::gnss::V2_0::renesas
