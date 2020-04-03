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

#define LOG_TAG "GnssRenesasHalConfigurator"
#define LOG_NDEBUG 1

#include <array>
#include <algorithm>
#include <stdexcept>
#include <log/log.h>
#include <functional>

#include "include/MessageQueue.h"
#include "include/Configurator.h"


namespace android::hardware::gnss::V2_0::renesas {

using cfg = Configurator;

const std::vector<std::vector<cfg::cfgStepPtr>> cfg::mConfigs = {
    {
        // protocol of init for u-blox FW version SPG 1.00
        &cfg::UbxClearConfig, &cfg::UbxSetNmea23, &cfg::UbxConfigGnssSGP100,
        &cfg::UbxSetNav5, &cfg::UbxSetPubx00, &cfg::UbxDisableNmeaGll,
        &cfg::UbxDisableNmeaVtg, &cfg::UbxSetRmc
    },
    {
        // protocol of init for u-blox FW version SPG 2.01
        &cfg::UbxClearConfig, &cfg::UbxSetNmea41, &cfg::UbxConfigGnssSGP201,
        &cfg::UbxSetNav5, &cfg::UbxSetPubx00, &cfg::UbxDisableNmeaGll,
        &cfg::UbxDisableNmeaVtg, &cfg::UbxPollNavGps, &cfg::UbxPollNavClock,
        &cfg::UbxPollRxmMeasx, &cfg::UbxPollNavStatus, &cfg::UbxSetRmc
    },
    {
        // protocol of init for u-blox FW version SPG 3.01
        &cfg::UbxClearConfig, &cfg::UbxSetNmea41, &cfg::UbxConfigGnssSGP301,
        &cfg::UbxSetNav5, &cfg::UbxSetPubx00, &cfg::UbxDisableNmeaGll,
        &cfg::UbxDisableNmeaVtg, &cfg::UbxPollNavGps, &cfg::UbxPollNavClock,
        &cfg::UbxPollRxmMeasx, &cfg::UbxPollNavStatus, &cfg::UbxSetRmc
    }
};


CError Configurator::Config() {
    ALOGV("Create Configurator");

    if (nullptr == mReceiver) {
        ALOGE("%s: No receiver", __func__);
        return CError::InternalError;
    }

    if (GnssVendor::Ublox == mReceiver->GetVendorId()) {
        return ConfigUbx();
    }

    ALOGE("%s: Unsupported receiver", __func__);
    return CError::UnsupportedReceiver;
}

CError Configurator::ConfigUbx() {
    ALOGV("Config Ubx");

    if (nullptr == mReceiver) {
        ALOGV("%s, mReceiver == null", __func__);
        return CError::InternalError;
    }
    MessageQueue& pipe = MessageQueue::GetInstance();
    pipe.Clear<std::shared_ptr<UbxACK>>();
    try {
        for (auto nextStep : mConfigs.at(mReceiver->GetSwVersion())) {
            if (auto res = std::invoke(nextStep, this);
                                    CError::Success != res) {
                return res;
            }
        }
    } catch (std::out_of_range e) {
        ALOGV("%s", e.what());
        return CError::UnsupportedReceiver;
    } catch (std::exception e) {
        ALOGV("%s", e.what());
        return CError::UnsupportedReceiver;
    }

    ALOGV("Config Success");
    return CError::Success;
}

CError Configurator::UbxClearConfig() {
    ALOGV("%s", __func__);
    return UbxSendRepeated<cfgClearLen>(cfgClear,
                                            UbxClass::CFG, UbxId::CLEAR);
}

CError Configurator::UbxSetNmea41() {
    ALOGV("%s", __func__);
    return UbxSendRepeated<cfgNmea41Len>(cfgNmea41,
                                            UbxClass::CFG, UbxId::NMEA);
}

CError Configurator::UbxSetNmea23() {
    ALOGV("%s", __func__);
    return UbxSendRepeated<cfgNmea23Len>(cfgNmea23,
                                            UbxClass::CFG, UbxId::NMEA);
}

CError Configurator::UbxSetNav5() {
    ALOGV("%s", __func__);
    return UbxSendRepeated<cfgNav5Len>(cfgNav5, UbxClass::CFG, UbxId::NAV5);
}

CError Configurator::UbxSetPubx00() {
    ALOGV("%s", __func__);
    return UbxSetMessageRate(UbxClass::NMEA_CFG_PUBX,
                        UbxId::NMEA_PUBX_POSITION, cfgDefaulRate);
}

CError Configurator::UbxConfigGnssSGP100() {
    ALOGV("%s", __func__);
    return UbxSendRepeated<cfgGnssSPG100Len>(cfgGnssSPG100, UbxClass::CFG,
            UbxId::GNSS);
}

CError Configurator::UbxConfigGnssSGP201() {
    ALOGV("%s", __func__);
    std::array<uint8_t, cfgGnssSPG201Len> msgCfgGnss = cfgGnssSPG201;
    PrepareGnssConfig(cfgIndexSPG201, msgCfgGnss);
    return UbxSendRepeated<cfgGnssSPG201Len>(msgCfgGnss, UbxClass::CFG,
            UbxId::GNSS);
    // //TODO(g.chabukiani): implement thin settings of sec major, sbas etc
}

CError Configurator::UbxConfigGnssSGP301() {
    ALOGV("%s", __func__);
    std::array<uint8_t, cfgGnssSPG301Len> msgCfgGnss = cfgGnssSPG301;
    PrepareGnssConfig(cfgIndexSPG301, msgCfgGnss);
    return UbxSendRepeated<cfgGnssSPG301Len>(msgCfgGnss, UbxClass::CFG,
            UbxId::GNSS);
    // //TODO(g.chabukiani): implement thin settings of sec major, sbas etc
}

CError Configurator::UbxDisableNmeaGll() {
    ALOGV("%s", __func__);
    return UbxSetMessageRate(UbxClass::NMEA_CFG, UbxId::NMEA_GLL,
                                                        cfgDisableRate);
}
CError Configurator::UbxDisableNmeaVtg() {
    ALOGV("%s", __func__);
    return UbxSetMessageRate(UbxClass::NMEA_CFG, UbxId::NMEA_VTG,
                                                        cfgDisableRate);
}
CError Configurator::UbxPollNavClock() {
    ALOGV("%s", __func__);
    return UbxSetMessageRate(UbxClass::NAV, UbxId::CLOCK, cfgDefaulRate);
}
CError Configurator::UbxPollNavGps() {
    ALOGV("%s", __func__);
    return UbxSetMessageRate(UbxClass::NAV, UbxId::TIMEGPS, cfgDefaulRate);
}
CError Configurator::UbxPollNavStatus() {
    ALOGV("%s", __func__);
    return UbxSetMessageRate(UbxClass::NAV, UbxId::STATUS, cfgDefaulRate);
}
CError Configurator::UbxPollRxmMeasx() {
    ALOGV("%s", __func__);
    return UbxSetMessageRate(UbxClass::RXM, UbxId::MEASX, cfgDefaulRate);
}
CError Configurator::UbxSetRmc() {
    ALOGV("%s", __func__);
    return UbxSetMessageRate(UbxClass::NMEA_CFG, UbxId::RMC, cfgDefaulRate);
}

//TODO(g.chabukiani): poll nav-pvt message,
//consider to use only ublox binary protocol for all needs of android hal iface

CError Configurator::UbxSetMessageRate(const UbxClass& _class,
                        const UbxId& _id, const uint8_t& rate) {
    ALOGV("%s", __func__);
    uint8_t reqClass = static_cast<uint8_t>(_class);
    uint8_t reqId = static_cast<uint8_t>(_id);
    //TODO(g.chabukiani): check the reason of setting rate 
    //in specific ports (0,1,3,4) of 6
    std::array<uint8_t, cfgSetRateLen> cfgSetRate = {
        0x06, 0x01, 0x08, 0x00, reqClass, reqId, rate, rate,
        0x00, rate, rate, 0x00
    };

    if (auto res = mTransport.Write<cfgSetRateLen>(cfgSetRate);
        TError::Success != res) {
        return CError::InternalError;
    }

    return CError::Success;
}

Configurator::Configurator(const std::shared_ptr<IGnssReceiver>& receiver) :
    mReceiver(receiver),
    mTransport(Transport::GetInstance(mReceiver)) {
    ALOGV("%s", __func__);
}

CError Configurator::WaitConfirmation(const UbxClass& _class,
                                      const UbxId& _id) {
    ALOGV("%s", __func__);
    MessageQueue& pipe = MessageQueue::GetInstance();
    std::condition_variable& cv = pipe.GetConditionVariable<CvAckType>();
    std::unique_lock<std::mutex> lock(mLock);
    const size_t timeout = 5000;
    cv.wait_for(lock, std::chrono::milliseconds{timeout},
                [&] {return !pipe.Empty<std::shared_ptr<UbxACK>>();});

    if (pipe.Empty<CvAckType>()) {
        return CError::InternalError;
    }

    std::shared_ptr<UbxACK> parcel = pipe.Pop<std::shared_ptr<UbxACK>>();
    return UbxProcessAck(_class, _id, parcel);
}

CError Configurator::UbxProcessAck(const UbxClass& _class, const UbxId& _id,
                                   const std::shared_ptr<UbxACK>& parcel) {
    if (nullptr == parcel) {
        return CError::InternalError;
    }

    std::pair<UbxClass, UbxId> ack;

    if (auto res = parcel->GetData(ack); UPError::Success != res) {
        return CError::InternalError;
    }

    if (UbxMsg::ACK_ACK == parcel->GetMsgType() && _class == ack.first
        && _id == ack.second) {
        ALOGV("%s, ACK", __func__);
        return CError::Success;
    }

    ALOGV("%s, NACK", __func__);
    return CError::InternalError;
}

void GetProp(char* secmajor, char* sbas) {
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    property_get("ro.boot.gps.secmajor", secmajor, "glonass");
    property_get("ro.boot.gps.sbas", sbas, "enable");

    std::transform(secmajor, secmajor + strlen(secmajor), secmajor, toupper);
    std::transform(sbas, sbas + strlen(sbas), sbas, toupper);
}

template <std::size_t size>
void PrepareGnssConfig(const std::array<uint8_t, CfgIndex::COUNT>& index,
            std::array<uint8_t, size>& ubxCfgGnss) {
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    char propSecmajor[PROPERTY_VALUE_MAX] = {};
    char propSbas[PROPERTY_VALUE_MAX] = {};

    GetProp(propSecmajor, propSbas);

    // Byte offset to set number of reserved (minimum)tracking channels for system
    const int CFG_GNSS_MIN = 1;
    // Byte offset to enable system
    const int CFG_GNSS_ENB = 4;

    bool sbasEnabled = true;
    if (strcmp(propSbas, "DISABLE") == 0) {
        sbasEnabled = false;
        // set minimum tracking channels for SBAS to zero
        ubxCfgGnss[index[CfgIndex::SIZE] * index[CfgIndex::SBAS] + CFG_GNSS_MIN] = 0x00;
        // disable SBAS
        ubxCfgGnss[index[CfgIndex::SIZE] * index[CfgIndex::SBAS] + CFG_GNSS_ENB] = 0x00;
    }
    ALOGV("Using GPS (major GNSS) and minor ones: GALILEO, QZSS%s",
            (sbasEnabled) ? ", SBAS" : "");
    if (strcmp(propSecmajor, "GLONASS") == 0) {
        // set minimum tracking channels for GLONASS to eight
        ubxCfgGnss[index[CfgIndex::SIZE] * index[CfgIndex::GLONASS] + CFG_GNSS_MIN] = 0x08;
        // enable GLONASS
        ubxCfgGnss[index[CfgIndex::SIZE] * index[CfgIndex::GLONASS] + CFG_GNSS_ENB] = 0x01;
        ALOGV("Using GLONASS as second major GNSS");
    } else if (strcmp(propSecmajor, "BEIDOU") == 0) {
        // set minimum tracking channels for BEIDOU to eight
        ubxCfgGnss[index[CfgIndex::SIZE] * index[CfgIndex::BEIDOU] + CFG_GNSS_MIN] = 0x08;
        // enable BEIDOU
        ubxCfgGnss[index[CfgIndex::SIZE] * index[CfgIndex::BEIDOU] + CFG_GNSS_ENB] = 0x01;
        ALOGV("Using BEIDOU as second major GNSS");
    } else {
        // by default the CFG GNSS message contains disabled GLONASS/BEIDOU part
        ALOGV("No second major GNSS is being used");
    }
}

} //namespace android::hardware::gnss::V2_0::renesas
