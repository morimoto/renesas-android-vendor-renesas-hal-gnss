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
#include "include/UbloxReceiver.h"

#ifdef BIG_ENDIAN_CPU
static inline uint32_t cpu_to_le32(uint32_t value) {
    return ((((value)&0xff000000u) >> 24) | (((value)&0x00ff0000u) >> 8) |
            (((value)&0x0000ff00u) << 8) | (((value)&0x000000ffu) << 24));
}
static inline uint16_t cpu_to_le16(uint32_t value) {
    return ((((value) >> 8) & 0xffu) | (((value)&0xffu) << 8));
}
#else
static inline uint32_t cpu_to_le32(uint32_t value) {
    return value;
}
static inline uint16_t cpu_to_le16(uint32_t value) {
    return value;
}
#endif

static inline uint32_t Encode8N1() {
    return (uint32_t)(1 << 11) | (3 << 6);
}

namespace android::hardware::gnss::V2_0::renesas {

using cfg = Configurator;

const std::vector<std::vector<cfg::cfgStepPtr>> cfg::mConfigs = {
    {
        // protocol of init for u-blox FW version SPG 1.00
        &cfg::UbxGnssReset,
        &cfg::UbxClearConfig, &cfg::UbxSetNmea23, &cfg::UbxConfigGnssSGP100,
        &cfg::UbxSetNav5, &cfg::UbxSetPubx00, &cfg::UbxDisableNmeaGll,
        &cfg::UbxDisableNmeaVtg, &cfg::UbxSetRmc
    },
    {
        // protocol of init for u-blox FW version SPG 2.01
        &cfg::UbxGnssReset,
        &cfg::UbxClearConfig, &cfg::UbxSetNmea41, &cfg::UbxConfigGnssSGP201,
        &cfg::UbxSetNav5, &cfg::UbxSetPubx00, &cfg::UbxDisableNmeaGll,
        &cfg::UbxDisableNmeaVtg, &cfg::UbxPollNavGps, &cfg::UbxPollNavClock,
        &cfg::UbxPollRxmMeasx, &cfg::UbxPollNavStatus, &cfg::UbxSetRmc
    },
    {
        // protocol of init for u-blox FW version SPG 3.01
        &cfg::UbxGnssReset,
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

    if (GnssVendor::Ublox == mReceiver->GetVendorId()) {
        mUbxReceiver = std::static_pointer_cast<UbloxReceiver>(mReceiver);
    }

    if (CError::Success != UbxChangeBaudRate()) {
        return CError::InternalError;
    }

    if (CError::Success != PollUbxMonVer()) {
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

    ALOGI("Config Success");
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
CError Configurator::UbxGnssReset() {
    ALOGV("%s", __func__);
    const uint8_t gnssReset = 0x02;  //Controlled software reset (GNSSonly)

    return UbxCfgRst(gnssReset);
}

CError Configurator::UbxSetSpeed(uint8_t port, uint32_t speed) {
    ALOGV("[%s, line %d]", __func__, __LINE__);
    std::array<uint8_t, ubxCfgDataLen + ubxCfgHeaderLen> msgUbloxCfgPrt = {};
    uint8_t* payload_buf = &msgUbloxCfgPrt[ubxCfgHeaderLen];

    // offset 0 has class code
    msgUbloxCfgPrt[0] = static_cast<uint8_t>(UbxClass::CFG);
    //offset 2 has 2 bytes of payload length
    *((uint16_t*)&msgUbloxCfgPrt[2]) = cpu_to_le16(ubxCfgDataLen);

    payload_buf[0] = port;
    *((uint32_t*)&payload_buf[4]) = cpu_to_le32(Encode8N1());
    *((uint32_t*)&payload_buf[8]) = cpu_to_le32(speed);

    /* enable NMEA and UBX protocols by default, we dont use RTCM protocol so far */
    const uint8_t UBX_PROTOCOL_MASK = 0x01;
    const uint8_t NMEA_PROTOCOL_MASK = 0x02;
    payload_buf[12] = NMEA_PROTOCOL_MASK | UBX_PROTOCOL_MASK;
    payload_buf[14] = NMEA_PROTOCOL_MASK | UBX_PROTOCOL_MASK;
    auto res = mReceiver->GetTransport()->Write<msgUbloxCfgPrt.size()>
               (msgUbloxCfgPrt);

    if (TError::Success != res) {
        return CError::InternalError;
    }

    if (TError::TransportReady !=
        mUbxReceiver->GetTransportTTY()->SetBaudRate(speed)) {
        ALOGE("Can not set tty baudrate to %d!\n", speed);
        return CError::InternalError;
    }

    if (CError::Success != WaitConfirmation(UbxClass::CFG, UbxId::NACK)) {
        ALOGW("No ACK for Speed Change! This is fatal\n");
        return CError::InternalError;
    }

    mUbxReceiver->GetTransportTTY()->SetBaudRate(speed);
    return CError::Success;
}

CError Configurator::UbxChangeBaudRate() {
    uint32_t gnssBaudrate = property_get_int32(propGnssBaudRate.c_str(),
                                               gnssDefaultRate);

    if (mUbxReceiver->GetTransportTTY()->GetBaudRate() != gnssBaudrate) {
        const uint8_t PortID = 1;

        if (CError::Success != UbxSetSpeed(PortID, gnssBaudrate)) {
            ALOGE("Can not set Gnss port baudrate to %d!\n", gnssBaudrate);
            return CError::InternalError;
        }

        ALOGD("Set gnss baudrate %u - success\n", gnssBaudrate);
    }

    return CError::Success;
}

CError Configurator::PollUbxMonVer() {
    ALOGV("%s", __func__);
    auto res = mReceiver->GetTransport()->Write<msgPollVerLen>(msgPollMonVer);

    if (TError::Success != res) {
        return CError::InternalError;
    }

    MessageQueue& pipe = MessageQueue::GetInstance();
    std::condition_variable& cv =
        pipe.GetConditionVariable<std::shared_ptr<IUbxParser<monVerOut>>>();
    std::unique_lock<std::mutex> lock(mLock);
    const size_t timeout = 5000;
    cv.wait_for(lock, std::chrono::milliseconds{timeout},
                [&] { return !pipe.Empty<std::shared_ptr<IUbxParser<monVerOut>>>(); });
    ALOGV("%s, ready", __func__);

    if (pipe.Empty<std::shared_ptr<IUbxParser<monVerOut>>>()) {
        ALOGE("%s, empty pipe", __func__);
        return CError::InternalError;
    }

    return ProcessUbxMonVer();
}

CError Configurator::ProcessUbxMonVer() {
    MessageQueue& pipe = MessageQueue::GetInstance();
    auto parcel = pipe.Pop<std::shared_ptr<IUbxParser<monVerOut>>>();
    monVerOut_t value = {};

    if (nullptr == parcel) {
        return CError::InternalError;
    }

    if (UPError::Success != parcel->GetData(value)) {
        return CError::InternalError;
    }

    auto res = mReceiver->SetFwVersion(value.swVersion);
    ALOGI("Firmware Version %.2f", mReceiver->GetFirmwareVersion());
    return (res == RError::NotSupported ? CError::UnsupportedReceiver : CError::Success);
}

CError Configurator::UbxCfgRst(uint8_t resetMode) {
    ALOGV("%s, line %d", __func__, __LINE__);
    std::array<uint8_t, cfgResetLen> cfgRstReset(cfgReset);

    // offset 2 has resetMode
    cfgRstReset[ubxCfgHeaderLen + 2] = resetMode;
    auto res = mReceiver->GetTransport()->Write<cfgResetLen>(cfgRstReset);

    if (TError::Success != res) {
        return CError::InternalError;
    }

    //Don't expect this message to be acknowledged by the receiver.
    usleep(25000);
    return CError::Success;
}

//TODO(g.chabukiani): poll nav-pvt message,
//consider to use only ublox binary protocol for all needs of android hal iface

CError Configurator::UbxSetMessageRate(const UbxClass& _class,
                        const UbxId& _id, const uint8_t& rate) {
    ALOGV("%s", __func__);
    uint8_t reqClass = static_cast<uint8_t>(_class);
    uint8_t reqId = static_cast<uint8_t>(_id);
    // TODO(g.chabukiani): check the reason of setting rate
    // in specific ports (0,1,3,4) of 6
    std::array<uint8_t, cfgSetRateLen> cfgSetRate = {
        0x06, 0x01, 0x08, 0x00, reqClass, reqId, rate, rate,
        0x00, rate, rate, 0x00
    };

    if (auto res = mReceiver->GetTransport()->Write<cfgSetRateLen>(cfgSetRate);
        TError::Success != res) {
        return CError::InternalError;
    }

    return CError::Success;
}

Configurator::Configurator(const std::shared_ptr<IGnssReceiver>& receiver) :
    mReceiver(receiver) {
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
