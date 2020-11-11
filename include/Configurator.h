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

#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <log/log.h>

#include <IGnssReceiver.h>
#include <IUbxParser.h>

namespace android::hardware::gnss::V2_1::renesas {

/**
 * @brief Reset message length
 */
static constexpr size_t cfgResetLen = 8;

/**
 * @brief Clear config message length
 */
static constexpr size_t cfgClearLen = 17;

/**
 * @brief cfgNav5Len
 */
static constexpr size_t cfgNav5Len = 40;

/**
 * @brief cfgNmea41Len
 */
static constexpr size_t cfgNmea41Len = 19;

/**
 * @brief cfgNmea23Len
 */
static constexpr size_t cfgNmea23Len = 16;

/**
 * @brief cfgGnssSPG100Len
 */
static constexpr size_t cfgGnssSPG100Len = 40;

/**
 * @brief cfgGnssSPG201Len
 */
static constexpr size_t cfgGnssSPG201Len = 48;

/**
 * @brief cfgGnssSPG301Len
 */
static constexpr size_t cfgGnssSPG301Len = 64;

/**
 * @brief cfgSetRateLen
 */
static constexpr size_t cfgSetRateLen = 12;

/**
 * @brief msgPollVerLen
 */
static constexpr size_t msgPollVerLen = 4;

/**
 * @brief ubx Cfg Data Length
 */
static constexpr size_t ubxCfgDataLen = 20;

/**
 * @brief ubx Cfg Header Length
 */
static constexpr size_t ubxCfgHeaderLen = 4;

/**
 * @brief maxAckRetries
 */
static constexpr uint8_t maxAckRetries = 5;

/**
 * @brief cfg Defaul Rate
 */
static constexpr uint8_t cfgDefaulRate = 1;

/**
 * @brief cfg Disable Rate
 */
static constexpr uint8_t cfgDisableRate = 0;

/**
 * @brief GNSS default baudrate
 */
static const int32_t gnssDefaultRate = 38400;

/**
 * @brief Configurator return type
 */
enum class CError : uint8_t {
    /**
     * @brief Success
     */
    Success,
    /**
     * @brief Internal Error
     */
    InternalError,
    /**
     * @brief Unsupported Receiver
     */
    UnsupportedReceiver,
};

/**
 * @brief Config Index
 */
enum CfgIndex : size_t {
    /**
     * @brief Size
     */
    SIZE = 0,
    /**
     * @brief SBAS
     */
    SBAS = 1,
    /**
     * @brief BEIDOU
     */
    BEIDOU = 2,
    /**
     * @brief GLONASS
     */
    GLONASS = 3,
    /**
     * @brief COUNT
     */
    COUNT = 4
};

class UbloxReceiver;

/**
 * @brief Configurator
 */
class Configurator {
public:
    /**
     * @brief Construct a new Configurator object
     *
     * @param receiver
     */
    Configurator(const std::shared_ptr<IGnssReceiver>& receiver);

    /**
     * @brief Destroy the Configurator object
     */
    ~Configurator() = default;

    /**
     * @brief Config
     *
     * @return CError
     */
    CError Config();
protected:
    /**
     * @brief ConfigUbx
     *
     * @return CError
     */
    CError ConfigUbx();

    /**
     * @brief UbxClearConfig
     *
     * @return CError
     */
    CError UbxClearConfig();

    /**
     * @brief UbxSetNmea41
     *
     * @return CError
     */
    CError UbxSetNmea41();

    /**
     * @brief UbxSetNmea23
     *
     * @return CError
     */
    CError UbxSetNmea23();

    /**
     * @brief UbxSetNav5
     *
     * @return CError
     */
    CError UbxSetNav5();

    /**
     * @brief UbxSetPubx00
     *
     * @return CError
     */
    CError UbxSetPubx00();

    /**
     * @brief UbxConfigGnssSGP100
     *
     * @return CError
     */
    CError UbxConfigGnssSGP100();

    /**
     * @brief UbxConfigGnssSGP201
     *
     * @return CError
     */
    CError UbxConfigGnssSGP201();

    /**
     * @brief UbxConfigGnssSGP301
     *
     * @return CError
     */
    CError UbxConfigGnssSGP301();

    /**
     * @brief UbxDisableNmeaGll
     *
     * @return CError
     */
    CError UbxDisableNmeaGll();

    /**
     * @brief UbxDisableNmeaVtg
     *
     * @return CError
     */
    CError UbxDisableNmeaVtg();

    /**
     * @brief UbxPollNavClock
     *
     * @return CError
     */
    CError UbxPollNavClock();

    /**
     * @brief UbxPollNavGps
     *
     * @return CError
     */
    CError UbxPollNavGps();

    /**
     * @brief UbxPollNavStatus
     *
     * @return CError
     */
    CError UbxPollNavStatus();

    /**
     * @brief UbxPollRxmMeasx
     *
     * @return CError
     */
    CError UbxPollRxmMeasx();

    /**
     * @brief UbxSetRmc
     *
     * @return CError
     */
    CError UbxSetRmc();

    /**
     * @brief UbxSetMessageRate
     *
     * @param _class
     * @param _id
     * @param rate
     * @return CError
     */
    CError UbxSetMessageRate(const UbxClass& _class, const UbxId& _id,
                             const uint8_t& rate);

    /**
     * @brief WaitConfirmation
     *
     * @param _class
     * @param _id
     * @return CError
     */
    CError WaitConfirmation(const UbxClass& _class, const UbxId& _id);

    /**
     * @brief UbxProcessAck
     *
     * @param _class
     * @param _id
     * @param type
     * @return CError
     */
    CError UbxProcessAck(const UbxClass& _class, const UbxId& _id,
                         const std::shared_ptr<UbxACK>& type);

    /**
     * \brief UbxChangeBaudRate
     * \return
     */
    CError UbxChangeBaudRate();

    /**
     * @brief
     *
     * @return CError
     */
    CError UbxGnssReset();

    /**
     * @brief
     *
     * @param port
     * @param speed
     * @return CError
     */
    CError UbxSetSpeed(uint8_t port, uint32_t speed);

    /**
     * \brief UbxSetSpeed
     * \param resetMode
     * \return CError
     */
    CError UbxCfgRst(uint8_t resetMode);

    /**
     * \brief PollUbxMonVer
     * \return CError
     */
    CError PollUbxMonVer();

    /**
     * \brief ProcessUbxMonVer
     * \return CError
     */
    CError ProcessUbxMonVer();
private:
    typedef CError (Configurator::*cfgStepPtr)();
    static const std::vector<std::vector<cfgStepPtr>> mConfigs;

    std::shared_ptr<IGnssReceiver> mReceiver;
    // Sub-class of mReceiver as U-Blox receiver
    std::shared_ptr<UbloxReceiver> mUbxReceiver;

    std::mutex mLock;

    template <std::size_t size>
    CError UbxSendRepeated(const std::array<uint8_t, size> msg,
                UbxClass msgClass, UbxId msgId) {
        for (auto i = 0; i < maxAckRetries; ++i) {
            if (auto res = mReceiver->GetTransport()->Write<size>(msg);
                TError::Success != res) {
                return CError::InternalError;
            }

            if (CError::Success == WaitConfirmation(msgClass, msgId)) {
                return CError::Success;
            }
        }

        ALOGE("Didn't get ACK message after %hhu retries", maxAckRetries);
        return CError::InternalError;
    }

};

/**
 * @brief PrepareGnssConfig
 *
 * @tparam size
 * @param index
 * @param ubxCfgGnss
 */
template <std::size_t size>
void PrepareGnssConfig(const std::array<uint8_t, CfgIndex::COUNT>& index,
            std::array<uint8_t, size>& ubxCfgGnss);

/**
 * @brief msgPollMonVer
 */
const std::array<uint8_t, msgPollVerLen> msgPollMonVer = {
    0x0a, 0x04, 0x00, 0x00
};

/**
 * @brief cfgReset
 */
const std::array<uint8_t, cfgResetLen> cfgReset = {
    0x06, 0x04, 0x04, 0x00, 0xFF, 0xFF, 0x00, 0x00
};

/**
 * @brief cfgClear
 */
const std::array<uint8_t, cfgClearLen> cfgClear = {
    0x06, 0x09, 0x0D, 0x00, 0xFE, 0xFF, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00,
    0x17
};

/**
 * @brief cfgNav5
 */
const std::array<uint8_t, cfgNav5Len> cfgNav5 = {
    0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x04, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
    0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00,
    0x5E, 0x01, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/**
 * @brief cfgNmea41
 */
const std::array<uint8_t, cfgNmea41Len> cfgNmea41 = {
    0x06, 0x17, 0x0F, 0x00, 0x20, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00
};

/**
 * @brief cfgNmea23
 */
const std::array<uint8_t, cfgNmea23Len> cfgNmea23 = {
    0x06, 0x17, 0x0C, 0x00, 0x20, 0x23, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01
};

/**
 * @brief cfgGnssSPG100
 */
const std::array<uint8_t, cfgGnssSPG100Len> cfgGnssSPG100 = {
    0x06, 0x3E, 0x24, 0x00, 0x00, 0x16, 0x16, 0x04,
    0x00, 0x04, 0xff, 0x00, 0x01, 0x00, 0x00, 0x00,  // GPS
    0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,  // SBAS
    0x05, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,  // QZSS
    0x06, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00   // GLONASS
};

/**
 * @brief cfgGnssSPG301
 */
const std::array<uint8_t, cfgGnssSPG301Len> cfgGnssSPG301 = {
    0x06, 0x3E, 0x3C, 0x00, 0x00, 0x20, 0x20, 0x07,
    0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01,  // GPS
    0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0x01,  // SBAS
    0x02, 0x04, 0x08, 0x00, 0x01, 0x00, 0x01, 0x01,  // GALILEO
    0x03, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01,  // BEIDOU
    0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x03,  // IMES
    0x05, 0x00, 0x03, 0x00, 0x01, 0x00, 0x01, 0x05,  // QZSS
    0x06, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x01, 0x01   // GLONASS
};

/**
 * @brief cfgGnssSPG201
 */
const std::array<uint8_t, cfgGnssSPG201Len> cfgGnssSPG201 = {
    0x06, 0x3E, 0x2C, 0x00, 0x00, 0x20, 0x20, 0x05,
    0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01,  // GPS
    0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0x01,  // SBAS
    0x03, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01,  // BEIDOU
    0x05, 0x00, 0x03, 0x00, 0x01, 0x00, 0x01, 0x05,  // QZSS
    0x06, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x01, 0x01   // GLONASS
};

/**
 * @brief cfgIndexSPG201
 */
const std::array<uint8_t, CfgIndex::COUNT> cfgIndexSPG201 = {
    6,  // entry size
    2,  // SBAS
    3,  // BEIDOU
    5   // GLONASS
};

/**
 * @brief cfgIndexSPG301
 */
const std::array<uint8_t, CfgIndex::COUNT> cfgIndexSPG301 = {
    8,  // entry size
    2,  // SBAS
    4,  // BEIDOU
    7   // GLONASS
};

}  //namespace android::hardware::gnss::V2_1::renesas

#endif // CONFIGURATOR_H
