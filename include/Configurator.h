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

#include <memory>
#include <cutils/properties.h>

#include "include/IGnssReceiver.h"
#include "include/IUbxParser.h"
#include "include/GnssTransport.h"

namespace android::hardware::gnss::V2_1::renesas {

static constexpr size_t cfgResetLen = 8;
static constexpr size_t cfgClearLen = 17;
static constexpr size_t cfgNav5Len = 40;
static constexpr size_t cfgNmea41Len = 19;
static constexpr size_t cfgNmea23Len = 16;
static constexpr size_t cfgGnssSPG100Len = 40;
static constexpr size_t cfgGnssSPG201Len = 48;
static constexpr size_t cfgGnssSPG301Len = 64;
static constexpr size_t cfgSetRateLen = 12;
static constexpr size_t msgPollVerLen = 4;
static constexpr size_t ubxCfgDataLen = 20;
static constexpr size_t ubxCfgHeaderLen = 4;

static constexpr uint8_t maxAckRetries = 5;

static constexpr uint8_t cfgDefaulRate = 1;
static constexpr uint8_t cfgDisableRate = 0;

static const int32_t gnssDefaultRate = 38400;
static const std::string propGnssBaudRate = "ro.boot.gps.gnss_baudrate";

enum class CError : uint8_t {
    Success,
    InternalError,
    UnsupportedReceiver,
};

enum CfgIndex : size_t {
    SIZE = 0,
    SBAS = 1,
    BEIDOU = 2,
    GLONASS = 3,
    COUNT = 4
};

class UbloxReceiver;

class Configurator {
public:
    Configurator(const std::shared_ptr<IGnssReceiver>& receiver);
    ~Configurator() = default;
    CError Config();
protected:
    CError ConfigUbx();
    CError UbxClearConfig();
    CError UbxSetNmea41();
    CError UbxSetNmea23();
    CError UbxSetNav5();
    CError UbxSetPubx00();
    CError UbxConfigGnssSGP100();
    CError UbxConfigGnssSGP201();
    CError UbxConfigGnssSGP301();
    CError UbxDisableNmeaGll();
    CError UbxDisableNmeaVtg();
    CError UbxPollNavClock();
    CError UbxPollNavGps();
    CError UbxPollNavStatus();
    CError UbxPollRxmMeasx();
    CError UbxSetRmc();
    CError UbxSetMessageRate(const UbxClass& _class, const UbxId& _id,
                             const uint8_t& rate);
    CError WaitConfirmation(const UbxClass& _class, const UbxId& _id);
    CError UbxProcessAck(const UbxClass& _class, const UbxId& _id,
                         const std::shared_ptr<UbxACK>& type);

    /*!
     * \brief UbxChangeBaudRate
     * \return
     */
    CError UbxChangeBaudRate();

    /*!
     * \brief UbxGnssReset
     * \return
     */
    CError UbxGnssReset();

    /*!
     * \brief UbxSetSpeed
     * \param port
     * \param speed
     * \return
     */
    CError UbxSetSpeed(uint8_t port, uint32_t speed);

    /*!
     * \brief UbxSetSpeed
     * \param resetMode
     * \return
     */
    CError UbxCfgRst(uint8_t resetMode);

    /*!
     * \brief PollUbxMonVer
     * \return
     */
    CError PollUbxMonVer();

    /*!
     * \brief ProcessUbxMonVer
     * \return
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

template <std::size_t size>
void PrepareGnssConfig(const std::array<uint8_t, CfgIndex::COUNT>& index,
            std::array<uint8_t, size>& ubxCfgGnss);

const std::array<uint8_t, msgPollVerLen> msgPollMonVer = {
    0x0a, 0x04, 0x00, 0x00
};

const std::array<uint8_t, cfgResetLen> cfgReset = {
    0x06, 0x04, 0x04, 0x00, 0xFF, 0xFF, 0x00, 0x00
};

const std::array<uint8_t, cfgClearLen> cfgClear = {
    0x06, 0x09, 0x0D, 0x00, 0xFE, 0xFF, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00,
    0x17
};

const std::array<uint8_t,  cfgNav5Len> cfgNav5 = {
    0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x04, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
    0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00,
    0x5E, 0x01, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const std::array<uint8_t, cfgNmea41Len> cfgNmea41 = {
    0x06, 0x17, 0x0F, 0x00, 0x20, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00
};

const std::array<uint8_t, cfgNmea23Len> cfgNmea23 = {
    0x06, 0x17, 0x0C, 0x00, 0x20, 0x23, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01
};

const std::array<uint8_t, cfgGnssSPG100Len> cfgGnssSPG100 = {
    0x06, 0x3E, 0x24, 0x00, 0x00, 0x16, 0x16, 0x04,
    0x00, 0x04, 0xff, 0x00, 0x01, 0x00, 0x00, 0x00,  // GPS
    0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,  // SBAS
    0x05, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,  // QZSS
    0x06, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00   // GLONASS
};

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

const std::array<uint8_t, cfgGnssSPG201Len> cfgGnssSPG201 = {
    0x06, 0x3E, 0x2C, 0x00, 0x00, 0x20, 0x20, 0x05,
    0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01,  // GPS
    0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0x01,  // SBAS
    0x03, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01,  // BEIDOU
    0x05, 0x00, 0x03, 0x00, 0x01, 0x00, 0x01, 0x05,  // QZSS
    0x06, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x01, 0x01   // GLONASS
};

const std::array<uint8_t, CfgIndex::COUNT> cfgIndexSPG201 = {
    6, // entry size
    2, // SBAS
    3, // BEIDOU
    5  // GLONASS
};
const std::array<uint8_t, CfgIndex::COUNT> cfgIndexSPG301 = {
    8, // entry size
    2, // SBAS
    4, // BEIDOU
    7  // GLONASS
};

} //namespace android::hardware::gnss::V2_0::renesas
