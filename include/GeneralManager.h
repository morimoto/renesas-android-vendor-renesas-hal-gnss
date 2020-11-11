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

#ifndef GENERALMANAGER_H
#define GENERALMANAGER_H

#include <Configurator.h>
#include <GnssInfoProvider.h>
#include <GnssMeasurement.h>
#include <ILocationProvider.h>
#include <IReader.h>
#include <NmeaMsgHandler.h>
#include <UbloxMsgHandler.h>

namespace android::hardware::gnss::V2_1::renesas {

/**
 * \brief General manager error return type
 */
enum class GMError : uint8_t {
    /**
     * \brief Success
     */
    SUCCESS,
    /**
     * \brief Internl error
     */
    INTERNAL,
    /**
     * \brief No receiver found
     */
    NO_RECEIVER,
    /**
     * \brief Other error
     */
    FAIL,
};

/**
 * \brief Gnss receiver status
 */
enum class GnssReceiverStatus : uint8_t {
    /**
     * \brief wait for receiver
     */
    WAIT_FOR_RECEIVER,
    /**
     * \brief receiver found
     */
    RECEIVER_FOUND,
    /**
     * \brief wait for setup
     */
    WAIT_FOR_SETUP,
    /**
     * \brief setup done
     */
    SETUP_DONE,
    /**
     * \brief wait for configuration
     */
    WAIT_FOR_CONFIG,
    /**
     * \brief configuration finished
     */
    CONFIG_DONE,
    /**
     * \brief ready
     */
    READY = CONFIG_DONE,
};

class DeviceScanner;
class GnssImpl;

/**
 * \brief General manager class
 *
 */
class GeneralManager {
public:
    /**
     * \brief GNSS callback ptr V1_0
     */
    using GnssCbPtr_1_0 = android::sp<android::hardware::gnss::V1_0::IGnssCallback>;

    /**
     * \brief GNSS callback ptr V1_1
     */
    using GnssCbPtr_1_1 = android::sp<android::hardware::gnss::V1_1::IGnssCallback>;

    /**
     * \brief GNSS callback ptr V2_0
     */
    using GnssCbPtr_2_0 = android::sp<android::hardware::gnss::V2_0::IGnssCallback>;

    /**
     * \brief GNSS callback ptr V2_1
     */
    using GnssCbPtr_2_1 = android::sp<android::hardware::gnss::V2_1::IGnssCallback>;

    /**
     * \brief GNSS hal implementation
     */
    using GnssHalImpl = android::hardware::gnss::V2_1::renesas::GnssImpl;

    /**
     * @brief Construct a new General Manager object
     */
    GeneralManager();
    /**
     * @brief Destroy the General Manager object
     */
    ~GeneralManager();

    /*!
     * \brief Run
     * \return
     */
    android::status_t Run();

    /*!
     * \brief SetCallbackV1_0
     * \param cb
     * \return
     */
    GMError SetCallbackV1_0(const GnssCbPtr_1_0& cb);

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
     * \brief SetCallbackV2_1
     * \param cb
     * \return
     */
    GMError SetCallbackV2_1(const GnssCbPtr_2_1& cb);

    /*!
     * \brief CleanUpCb
     * \return
     */
    GMError CleanUpCb();

    /**
     * @brief Set the Update Period object
     *
     * @param updateIntervalMs
     */
    void setUpdatePeriod(uint32_t updateIntervalMs);

    /**
     * @brief Get the ExtensionGnssMeasurement v2 0 object
     *
     * @return sp<android::hardware::gnss::V2_0::IGnssMeasurement>
     */
    sp<android::hardware::gnss::V2_0::IGnssMeasurement> getExtensionGnssMeasurement_v2_0();

    /**
     * @brief Get the ExtensionGnssMeasurement v2 1 object
     *
     * @return sp<android::hardware::gnss::V2_1::IGnssMeasurement>
     */
    sp<android::hardware::gnss::V2_1::IGnssMeasurement> getExtensionGnssMeasurement_v2_1();

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

    /*!
     * \brief IsRun - return true if hal is running
     * \return
     */
    bool IsRun();

    /*!
     * \brief setGnssVisibilityControl - set gnss visibility control
     * pointer in location provider
     * \param gnssVisibilityControl gnss visibility control ptr
     * \return
     */
    void setGnssVisibilityControl(sp<GnssVisibilityControlV1_0>& gnssVisibilityControl);

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

    /**
     * @brief Setup SvInfo Provider
     *
     * @return GMError
     */
    GMError SetupSvInfoProvider();

private:
    GeneralManager(const GeneralManager&) = delete;
    GeneralManager operator=(const GeneralManager&) = delete;

    NmeaVersion GetNmeaProtocol();
    void GnssCriticalError(std::string what);
    void UpdateReceiverInfo();
    void RunConfig();

    GnssCbPtr_1_0 mGnssCallback_1_0;
    GnssCbPtr_1_1 mGnssCallback_1_1;
    GnssCbPtr_2_0 mGnssCallback_2_0;
    GnssCbPtr_2_1 mGnssCallback_2_1;
    std::shared_ptr<android::hardware::gnss::V2_1::renesas::GnssImpl> mGnssImpl;

    std::unique_ptr<DeviceScanner> mDeviceScanner;
    std::shared_ptr<IGnssReceiver> mReceiver;
    std::unique_ptr<IReader> mReader;
    std::unique_ptr<ILocationProvider> mLocationProvider;
    std::unique_ptr<GnssInfoProvider> mSvInfoProvider;
    sp<GnssMeasurement> mGnssMeasurement;
    std::unique_ptr<UbxMsgHandler> mUbxMsgHandler;
    std::unique_ptr<NmeaMsgHandler> mNmeaMsgHandler;

    uint32_t mUpdateIntervalUs = 1000 * 1000;  // set default interval to 1 sec

    GnssReceiverStatus mReceiverStatus;
    std::thread mReceiverInfoThread;
    std::thread mConfigThread;
    std::condition_variable mReceiverCv;
    std::mutex mLock;
    bool mIsRun;

    template <typename CbPtr>
    GMError FillCallback(const CbPtr& inputCallback, CbPtr& callback) {
        if (!inputCallback) {
            ALOGE("%s: Callback is null", __func__);
            return GMError::FAIL;
        }

        callback = inputCallback;
        callback->gnssSetCapabilitesCb(
            android::hardware::gnss::V2_0::IGnssCallback::Capabilities::GEOFENCING |
            android::hardware::gnss::V2_0::IGnssCallback::Capabilities::NAV_MESSAGES |
            android::hardware::gnss::V2_0::IGnssCallback::Capabilities::MEASUREMENTS);
        if (nullptr != mReceiver) {
            callback->gnssSetSystemInfoCb(
                {static_cast<uint16_t>(mReceiver->GetYearOfHw())});
        }
        callback->gnssStatusCb(android::hardware::gnss::V1_0::IGnssCallback::GnssStatusValue::NONE);

        return GMError::SUCCESS;
    }
};

}  //namespace android::hardware::gnss::V2_1::renesas

#endif  // GENERALMANAGER_H
