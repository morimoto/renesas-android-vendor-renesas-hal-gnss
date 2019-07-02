/*
 * Copyright (C) 2017 GlobalLogic
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

#define LOG_TAG "GnssRenesasHAL"
#define LOG_NDEBUG 1

#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>
#include <memory.h>
#include <stdio.h>
#include <sys/param.h> /* for MIN(x,y) */
#include <cstdlib>
#include <algorithm>
#include <memory>

#include <utils/SystemClock.h>
#include <sys/system_properties.h>

#include <android-base/logging.h>
#include <log/log.h>
#include <cutils/properties.h>

#include "GnssHw.h"
#include "GnssMeasToLocSync.h"
#include "GnssRxmMeasxParser.h"
#include "GnssNavClockParser.h"
#include "GnssNavTimeGPSParser.h"
#include "GnssNavStatusParser.h"
#include "GnssMeasQueue.h"
#include "UsbHandler.h"

static const uint16_t ublox7 = 0x01a7;
static const uint16_t ublox8 = 0x01a8;

static const uint8_t classUbxNav = 0x01;
static const uint8_t classUbxRxm = 0x02;
static const uint8_t classNmeaCfg = 0xF0;

static const uint8_t idClock = 0x22;
static const uint8_t idMeasx = 0x14;
static const uint8_t idTimeGps = 0x20;
static const uint8_t idStatus = 0x03;
static const uint8_t idRMC = 0x04;

static const uint8_t rate = 0x01;
static const uint8_t rateRMC = 0x01;

// According to u-blox M8 Receiver Description - Manual, UBX-13003221, R16 (5.11.2018),  32.2.14 RMC, p. 124
// NMEA protocol version 4.1 and above has 14 fields.
static const size_t rmcFieldsNumberNMEAv41 = 14;
static const size_t rmcFieldsNumberNMEAv23 = 13;

static const size_t ggaFieldsNumber = 15; // According to u-blox M8 Receiver Description - Manual, UBX-13003221, R16 (5.11.2018),  32.2.4 GGA, p. 114
static const size_t gsaFieldsNumberNMEAv41 = 19;
static const size_t gsaFieldsNumberNMEAv23 = 18;

static const size_t pubxFieldsNumber = 21;
static const size_t gsvFieldsNumber = 4;

static const size_t ackNackMsgLen = 2;
static const size_t ackNackClassOffset = 0;
static const size_t ackNackIdOffset = 1;

static const float speedAccUblox7 = 0.1f; // m/s according to NEO-7 datasheet
static const float bearingAccUblox7 = 0.5f; // degrees according to NEO-7 datasheet
static const float speedAccUblox8 = 0.05f; // m/s according to NEO-8 datasheet
static const float bearingAccUblox8 = 0.3f; // degrees according to NEO-8 datasheet

static const std::string ttyUsbDefault("/dev/ttyACM0");
static const std::string ttyDefaultKf("/dev/ttySC3");

static const uint8_t ublox_cfg_clear[] = {0x06, 0x09, 0x0D, 0x00, 0xFF, 0xFF, 0x00, 0x00,
                                          0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
                                          0x17};

static const uint8_t ublox_nav5[] = {0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x04, 0x02,
                                     0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
                                     0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00,
                                     0x5E, 0x01, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00,
                                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static const uint8_t ublox_enable_nmea41[] = {0x06, 0x17, 0x0F, 0x00, 0x20, 0x41,
                                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                              0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};

static const uint8_t ublox_enable_nmea23[] = {0x06, 0x17, 0x0C, 0x00, 0x20, 0x23,
                                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                              0x01, 0x00, 0x00, 0x01};

static void GetProp(char* secmajor, char* sbas);

template <typename mPtr>
static inline void CheckNotNull(mPtr val, const char* msg)
{
    if (nullptr == val) {
        ALOGE("%s", msg);
        std::abort();
    }
}

GnssHwIface::~GnssHwIface(void)
{
    mThreadExit = true;

    if (mThread.joinable()) {
        mThread.join();
    }
}

GnssHwTTY::GnssHwTTY()
{
    ALOGV("GnssHwTTY default constructor");
}

GnssHwTTY::GnssHwTTY(int fd) :
    mFd(fd),
    mEnabled(false),
    mHelpThreadExit(false),
    mUbxAckReceived(0)
{
    memset(&mGnssLocation, 0, sizeof(GnssLocation));
    memset(&mSvStatus, 0, sizeof(IGnssCallback::GnssSvStatus));
    mNmeaBuffer     = new(std::nothrow) CircularBuffer<NmeaBufferElement   >(32, sizeof(NmeaBufferElement));
    mUbxBuffer      = new(std::nothrow) CircularBuffer<UbxBufferElement    >(32, sizeof(UbxBufferElement));
    mUbxStateBuffer = new(std::nothrow) CircularBuffer<UbxStateQueueElement>(64, sizeof(UbxStateQueueElement));

    CheckNotNull(mNmeaBuffer, "Failed to allocate buffers");
    CheckNotNull(mUbxBuffer, "Failed to allocate buffers");
    CheckNotNull(mUbxStateBuffer, "Failed to allocate buffers");

    if (CheckHwPropertyKf()) {
        ALOGV("[%s, line %d] Kingfisher", __func__, __LINE__);
        CHECK_EQ(1, OpenDevice(ttyDefaultKf.c_str())) << "Failed to open device";

        mIsUbloxDevice = true;
        RunWorkerThreads();
    }
}

void GnssHwTTY::RunWorkerThreads()
{
    if (!mHwInitThread.joinable()) {
        mHwInitThread = std::thread(&GnssHwTTY::GnssHwUbxInitThread, this);
    }
    if (!mUbxThread.joinable()) {
        mUbxThread  = std::thread(&GnssHwTTY::UBX_Thread, this);
    }
    if (!mNmeaThread.joinable()) {
        mNmeaThread = std::thread(&GnssHwTTY::NMEA_Thread, this);
    }
}

GnssHwTTY::~GnssHwTTY(void)
{
    mHelpThreadExit.store(true);
    if (mIsUbloxDevice) {
        JoinWorkerThreads();
    }

    delete mNmeaBuffer;
    delete mUbxBuffer;
    delete mUbxStateBuffer;
}

void GnssHwTTY::resetOnStart()
{
        ALOGV("[%s, line %d] Reset HW", __func__, __LINE__);

        uint8_t ublox_cfg_reset[] = {0x06, 0x04, 0x04, 0x00, 0xFF, 0xFF, 0x02, 0x00};
        UBX_Send(ublox_cfg_reset, sizeof(ublox_cfg_reset));

        mResetReceiverOnStart = false;
}

bool GnssHwTTY::start(void)
{
    if (mResetReceiverOnStart) {
        resetOnStart();
    }

    ALOGD("Start HW");
    if (mIsKingfisher) {
        mEnabled = true;
    } else {
        mEnabled = StartSalvatorProcedure();
    }

    if (mGnssCb != nullptr) {
        mGnssCb->gnssStatusCb(IGnssCallback::GnssStatusValue::SESSION_BEGIN);
    }

    return mEnabled;
}

bool GnssHwTTY::stop(void)
{
    ALOGD("Stop HW");
    mEnabled = false;
    if (mGnssCb != nullptr) {
        mGnssCb->gnssStatusCb(IGnssCallback::GnssStatusValue::SESSION_END);
    }

    return true;
}

bool GnssHwTTY::OpenDevice(const char* ttyDevDefault)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    char prop_tty_dev[PROPERTY_VALUE_MAX] = {};
    char prop_tty_baudrate[PROPERTY_VALUE_MAX] = {};

    property_get("ro.boot.gps.tty_dev", prop_tty_dev, ttyDevDefault);
    property_get("ro.boot.gps.tty_baudrate", prop_tty_baudrate, "9600");

    int32_t baudrate = std::atoi(prop_tty_baudrate);

    /* Open the serial tty device */
    do {
        mFd = ::open(prop_tty_dev, O_RDWR | O_NOCTTY);
    } while (mFd < 0 && errno == EINTR);

    if (mFd < 0) {
        ALOGE("Could not open TTY device");
        return false;
    }

    mHandleThreadCv.notify_one();

    ALOGI("TTY %s@%s fd=%d", prop_tty_dev, prop_tty_baudrate, mFd);

    /* Setup serial port */
    struct termios  ios;
    ::tcgetattr(mFd, &ios);

    ios.c_cflag  = CS8 | CLOCAL | CREAD;
    ios.c_iflag  = IGNPAR;
    ios.c_oflag  = 0;
    ios.c_lflag  = 0;  /* disable ECHO, ICANON, etc... */

    /* Set baudrate */
    switch(baudrate) {
    case 2400: ios.c_cflag |= B2400; break;
    case 4800: ios.c_cflag |= B4800; break;
    case 9600: ios.c_cflag |= B9600; break;
    case 19200: ios.c_cflag |= B19200; break;
    case 38400: ios.c_cflag |= B38400; break;
    default: {
        ios.c_cflag |= B9600;
        ALOGW("Unsupported baud rate %d.. setting default 9600", baudrate);
    }
    }

    ::tcsetattr(mFd, TCSANOW, &ios);
    ::tcflush(mFd, TCIOFLUSH);

    return true;
}

bool GnssHwTTY::StartSalvatorProcedure()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    bool retStatus = true;
    mHelpThreadExit.store(false);


    if (!mIsUbloxDevice) {
        retStatus = OpenDevice(ttyUsbDefault.c_str());
        if (!retStatus) {
            ALOGE("Failed to open device");
            return retStatus;
        }

        retStatus = CheckUsbDeviceVendorUbx();
        if (!retStatus) {
            ALOGE("No u-blox device connected");
            return retStatus;
        }

        ALOGV("[%s, line %d] u-blox device", __func__, __LINE__);
        mIsUbloxDevice = retStatus;
        SetYearOfHardware();
        if (mGnssCb != nullptr) {
            mGnssCb->gnssSetSystemInfoCb({mYearOfHardware});
        }

        RunWorkerThreads();
    }

    return retStatus;
}

void GnssHwTTY::JoinWorkerThreads()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (mHwInitThread.joinable()) {
        mHwInitThread.join();
    }
    if (mUbxThread.joinable()) {
        mUbxThreadCv.notify_all();
        mUbxThread.join();
    }
    if (mNmeaThread.joinable()) {
        mNmeaThreadCv.notify_all();
        mNmeaThread.join();
    }

    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

void GnssHwTTY::ClearConfig()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    UBX_Send(ublox_cfg_clear, sizeof(ublox_cfg_clear));
    UBX_Wait(UbxRxState::WAITING_ACK, "Failed to clear UBX config", mUbxTimeoutMs);
}

void GnssHwTTY::SetNMEA41()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    // Enabling NMEA 4.1 Extended satellite numbering, set flag freeze bearing
    UBX_Send(ublox_enable_nmea41, sizeof(ublox_enable_nmea41));
    UBX_Wait(UbxRxState::WAITING_ACK, "Failed to set NMEA version to 4.1", mUbxTimeoutMs);

    //SetUp length of messages according to protocol version
    mRmcFieldsNumber = rmcFieldsNumberNMEAv41;
    mGsaFieldsNumber = gsaFieldsNumberNMEAv41;
}

void GnssHwTTY::SetNMEA23()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    // Enabling NMEA 2.3 Extended satellite numbering, set flag freeze bearing
    UBX_Send(ublox_enable_nmea23, sizeof(ublox_enable_nmea23));
    UBX_Wait(UbxRxState::WAITING_ACK, "Failed to set NMEA version to 2.3", mUbxTimeoutMs);

    //SetUp length of messages according to protocol version
    mRmcFieldsNumber = rmcFieldsNumberNMEAv23;
    mGsaFieldsNumber = gsaFieldsNumberNMEAv23;
}

static void GetProp(char* secmajor, char* sbas)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    if (nullptr == secmajor || nullptr == sbas) {
        ALOGV("[%s, line %d] Bad input", __func__, __LINE__);
        return;
    }
    property_get("ro.boot.gps.secmajor", secmajor, "glonass");
    property_get("ro.boot.gps.sbas", sbas, "enable");

    auto prop_secmajor_len = strnlen(secmajor, PROPERTY_VALUE_MAX);
    auto prop_sbas_len = strnlen(sbas, PROPERTY_VALUE_MAX);

    for (size_t i = 0; i < prop_secmajor_len; i++) {
        secmajor[i] = static_cast<char>(toupper(secmajor[i]));
    }

    for (size_t i = 0; i < prop_sbas_len; i++) {
        sbas[i] = static_cast<char>(toupper(sbas[i]));
    }
}

void GnssHwTTY::ConfigGnssUblox7()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    char prop_secmajor[PROPERTY_VALUE_MAX] = {};
    char prop_sbas[PROPERTY_VALUE_MAX] = {};

    GetProp(prop_secmajor, prop_sbas);

    //    CFG-GNSS example
    //    ID    - GNSS ID
    //    MN    - minimum (reserved) tracking channels for this GNSS
    //    MX    - maximum tracking channels
    //    XX    - reserved (always zero)
    //    F1-F4 - bitfield flags

    //              ID MN MX XX F1 F2 F3 F4
    //             ________________________
    //    GPS     | 00 08 10 00 01 00 00 00
    //    SBAS    | 01 01 03 00 01 00 00 00
    //    QZSS    | 06 00 03 00 01 00 00 00
    //    GLONASS | 06 08 0E 00 00 00 00 00

    const size_t disableSBASOffset = 20;
    const size_t mnSBASOffset = 17;

    uint8_t ublox_cfg_gnss[] = {0x06, 0x3E, 0x24, 0x00, 0x00, 0x16, 0x16, 0x04,
                                0x00, 0x04, 0xff, 0x00, 0x01, 0x00, 0x00, 0x00,  // GPS
                                0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,  // SBAS
                                0x05, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,  // QZSS
                                0x06, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00}; //GLONASS

    bool sbas_enabled = true;
    if (strncmp(prop_sbas, "DISABLE", PROPERTY_VALUE_MAX) == 0) {
        sbas_enabled = false;
        ublox_cfg_gnss[mnSBASOffset] = 0x00; // set minimum tracking channels for SBAS to zero
        ublox_cfg_gnss[disableSBASOffset] = 0x00; // disable SBAS
    }

    ALOGI("No second major GNSS is being used");
    mMajorGnssStatus = MajorGnssStatus::GPS_ONLY;

    UBX_Send(ublox_cfg_gnss, sizeof(ublox_cfg_gnss));
    UBX_Wait(UbxRxState::WAITING_ACK, "Failed to switch GNSS", mUbxTimeoutMs);
}

void GnssHwTTY::PrepareGnssConfig(char* propSecmajor, char* propSbas, uint8_t ubxCfgGnss[64])
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    if (nullptr == propSecmajor || nullptr == propSbas || nullptr == ubxCfgGnss) {
        ALOGV("[%s, line %d] Wrong input", __func__, __LINE__);
        return;
    }

    const int CFG_GNSS_ENTRY_SIZE = 8;
    const int CFG_GNSS_SBAS    = 2;
    const int CFG_GNSS_BEIDOU  = 4;
    const int CFG_GNSS_GLONASS = 7;

    const int CFG_GNSS_MIN = 1;
    const int CFG_GNSS_ENB = 4;

    bool sbasEnabled = true;
    if (strncmp(propSbas, "DISABLE", PROPERTY_VALUE_MAX) == 0) {
        sbasEnabled = false;
        ubxCfgGnss[CFG_GNSS_ENTRY_SIZE * CFG_GNSS_SBAS + CFG_GNSS_MIN] = 0x00; // set minimum tracking channels for SBAS to zero
        ubxCfgGnss[CFG_GNSS_ENTRY_SIZE * CFG_GNSS_SBAS + CFG_GNSS_ENB] = 0x00; // disable SBAS
    }

    ALOGI("Using GPS (major GNSS) and minor ones: GALILEO, QZSS%s", (sbasEnabled) ? ", SBAS" : "");
    if (strncmp(propSecmajor, "GLONASS", PROPERTY_VALUE_MAX) == 0) {
        ubxCfgGnss[CFG_GNSS_ENTRY_SIZE * CFG_GNSS_GLONASS + CFG_GNSS_MIN] = 0x08; // set minimum tracking channels for GLONASS to eight
        ubxCfgGnss[CFG_GNSS_ENTRY_SIZE * CFG_GNSS_GLONASS + CFG_GNSS_ENB] = 0x01; // enable GLONASS
        ALOGI("Using GLONASS as second major GNSS");
        mMajorGnssStatus = MajorGnssStatus::GPS_GLONASS;
    } else if (strncmp(propSecmajor, "BEIDOU", PROPERTY_VALUE_MAX) == 0) {
        ubxCfgGnss[CFG_GNSS_ENTRY_SIZE * CFG_GNSS_BEIDOU + CFG_GNSS_MIN] = 0x08; // set minimum tracking channels for BEIDOU to eight
        ubxCfgGnss[CFG_GNSS_ENTRY_SIZE * CFG_GNSS_BEIDOU + CFG_GNSS_ENB] = 0x01; // enable BEIDOU
        ALOGI("Using BEIDOU as second major GNSS");
        mMajorGnssStatus = MajorGnssStatus::GPS_BEIDOU;
    } else {
        // No need to change anything, by default the CFG GNSS message contains disabled GLONASS/BEIDOU part
        ALOGI("No second major GNSS is being used");
        mMajorGnssStatus = MajorGnssStatus::GPS_ONLY;
    }
}

void GnssHwTTY::ConfigGnssUblox8()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    char propSecmajor[PROPERTY_VALUE_MAX] = {};
    char propSbas[PROPERTY_VALUE_MAX] = {};

    GetProp(propSecmajor, propSbas);

    //    CFG-GNSS example
    //    ID    - GNSS ID
    //    MN    - minimum (reserved) tracking channels for this GNSS
    //    MX    - maximum tracking channels
    //    XX    - reserved (always zero)
    //    F1-F4 - bitfield flags

    //              ID MN MX XX F1 F2 F3 F4
    //             ________________________
    //    GPS     | 00 08 10 00 01 00 01 01
    //    SBAS    | 01 01 03 00 01 00 01 01
    //    GALILEO | 02 04 08 00 00 00 01 01
    //    BEIDOU  | 03 08 10 00 00 00 01 01
    //    IMES    | 04 00 08 00 00 00 01 03
    //    QZSS    | 05 00 03 00 01 00 01 05
    //    GLONASS | 06 08 0E 00 01 00 01 01

    uint8_t ubloxCfgGnss[] = {0x06, 0x3E, 0x3C, 0x00, 0x00, 0x20, 0x20, 0x07,
                              0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01,  // GPS
                              0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0x01,  // SBAS
                              0x02, 0x04, 0x08, 0x00, 0x01, 0x00, 0x01, 0x01,  // GALILEO
                              0x03, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01,  // BEIDOU
                              0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x03,  // IMES
                              0x05, 0x00, 0x03, 0x00, 0x01, 0x00, 0x01, 0x05,  // QZSS
                              0x06, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x01, 0x01}; // GLONASS

    PrepareGnssConfig(propSecmajor, propSbas, ubloxCfgGnss);

    UBX_Send(ubloxCfgGnss, sizeof(ubloxCfgGnss));
    UBX_Wait(UbxRxState::WAITING_ACK, "Failed to switch GNSS", mUbxTimeoutMs);
}

void GnssHwTTY::PollCommonMessages()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    UBX_Send(ublox_nav5, sizeof(ublox_nav5));
    UBX_Wait(UbxRxState::WAITING_ACK, "Failed to set NAV5 to automotive", mUbxTimeoutMs);

    UBX_SetMessageRate(0xF1, 0x00, 1, "Failed to enable PUBX,00 message"); // enable PUBX,00
    UBX_SetMessageRate(0xF0, 0x01, 0, nullptr); // disable GLL
    UBX_SetMessageRate(0xF0, 0x05, 0, nullptr); // disable VTG

    UBX_SetMessageRateCurrentPort(classUbxNav, idClock, rate, "UBX-NAV-CLOCK config failed");
    UBX_SetMessageRateCurrentPort(classUbxNav, idTimeGps, rate, "UBX-NAV-GPS-TIME config failed");
    UBX_SetMessageRateCurrentPort(classUbxNav, idStatus, rate, "UBX-NAV-STATUS config failed");

    UBX_SetMessageRate(classNmeaCfg, idRMC, rateRMC, nullptr); // reduce RMC rate

    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

void GnssHwTTY::SetConstValuesOfHardware(uint16_t gen)
{
    switch (gen) {
    case ublox7: {
        mSpeedAcc = speedAccUblox7;
        mBearingAcc = bearingAccUblox7;
        break;
    }
    case ublox8: {
        mSpeedAcc = speedAccUblox8;
        mBearingAcc = bearingAccUblox8;
        break;
    }
    default: {
        mSpeedAcc = 0;
        mBearingAcc = 0;
    }
    }
}

void GnssHwTTY::InitUblox7Gen()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    SetConstValuesOfHardware(ublox7);
    ClearConfig();
    SetNMEA23();
    ConfigGnssUblox7();
    PollCommonMessages();
}

void GnssHwTTY::InitUblox8Gen()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    SetConstValuesOfHardware(ublox8);
    ClearConfig();
    SetNMEA41();
    ConfigGnssUblox8();
    PollCommonMessages();

    UBX_SetMessageRateCurrentPort(classUbxRxm, idMeasx, rate, "UBX-RXM-MEASX config failed");
}

void GnssHwTTY::GnssHwUbxInitThread(void)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    UBX_Reset();
    switch (mUbxGeneration) {
    case ublox7: {
        InitUblox7Gen();
        break;
    }
    case ublox8: {
        InitUblox8Gen();
        break;
    }
    default: {
        ALOGE("Unexpected Ublox receiver generation");
        UBX_CriticalProtocolError("UBX protocol failure, unknown ublox generation");
    }

    }
    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

void GnssHwTTY::GnssHwHandleThread(void)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    while (!mThreadExit) {
        if (mFd == -1) {
            std::unique_lock<std::mutex> lock(mHandleThreadLock);
            mHandleThreadCv.wait(lock);
            continue;
        }

        uint8_t ch;
        ssize_t ret = read(mFd, &ch, sizeof(ch));

        if (ret < 0 && errno == EAGAIN) {
            continue;
        }

        if (ret > 0) {
            ReaderPushChar(ch);
        } else {
            ALOGE("TTY read error: %s", strerror(errno));
            mFd  = -1;
            usleep(50000);
            continue;
        }
    }

    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

void GnssHwTTY::ReaderPushChar(unsigned char ch)
{
    if (mReaderBufPos >= sizeof(mReaderBuf)) {
        mReaderBufPos = 0;
        mReaderState = ReaderState::WAITING;
    }

    static uint16_t ubx_payload_len;

    if (ch == '$' && mReaderState == ReaderState::WAITING) {
        mReaderState = ReaderState::CAPTURING_NMEA;
    }
    else if (ch == mUbxSync1 && mReaderState == ReaderState::WAITING) {
        mReaderState = ReaderState::WAITING_UBX_SYNC2;
        ALOGV("UBX waiting sync2");
    }
    else if (mReaderState == ReaderState::CAPTURING_NMEA) {
        if (ch == '$') {
            /* Missed end of message CR LF ? Reset the reader */
            mReaderBufPos = 0;
        } else if (ch == '\r' || ch == '\n') {
            /* End of message */
            mReaderBuf[mReaderBufPos] = 0;

            /* Parse NMEA */
            mNmeaBuffer->put(mReaderBuf);

            mNmeaThreadCv.notify_all();

            /* Reset the reader  */
            mReaderBufPos = 0;
            mReaderState = ReaderState::WAITING;
            return;
        }
    }
    else if (mReaderState == ReaderState::WAITING_UBX_SYNC2) {
        if (ch == mUbxSync2) {
            ALOGV("UBX capturing");
            mReaderState = ReaderState::CAPTURING_UBX;
        }
    }
    else if (mReaderState == ReaderState::CAPTURING_UBX) {
        if (mReaderBufPos == mUbxLengthFirstByteNo) {
            ALOGV("UBX rx payload len byte1: %02X", ch);
            ubx_payload_len += ch;
        }
        else if (mReaderBufPos == mUbxLengthSecondByteNo) {
            ubx_payload_len += ch << 8;
            ALOGV("UBX rx payload len byte2: %02X", ch);
            ALOGV("UBX rx payload len: %d", ubx_payload_len);
        }
    }

    ALOGV("[%s, line %d] waiting", __func__, __LINE__);
    if (mReaderState != ReaderState::WAITING) {
        mReaderBuf[mReaderBufPos++] = ch;

        if (mReaderState == ReaderState::CAPTURING_UBX &&
                mReaderBufPos >= (ubx_payload_len + mUbxPacketSizeNoPayload)) {

            /* Parse UBX */
            ALOGV("UBX rx putting buffer len: %zu", mReaderBufPos);

            UbxBufferElement elem;
            elem.len = mReaderBufPos;
            if (elem.len < (sizeof(UbxBufferElement) - sizeof(size_t))) {
                ALOGV("[%s, line %d] add, notify all", __func__, __LINE__);
                memcpy(&elem.data, mReaderBuf, elem.len);
                mUbxBuffer->put(&elem);
                mUbxThreadCv.notify_all();
            } else {
                ALOGE("Received UBX message that is too large for the buffer (%lu)", elem.len);
            }

            /* Reset the reader  */
            mReaderBufPos = 0;
            mReaderState = ReaderState::WAITING;
            ubx_payload_len = 0;
        }
    }
}

void GnssHwTTY::NMEA_Thread(void)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    while (!mHelpThreadExit) {
        if (!mNmeaBuffer->empty()) {
            NMEA_ReaderParse(&(mNmeaBuffer->get()->data[0]));
        } else {
            std::unique_lock<std::mutex> lock(mNmeaThreadLock);
            mNmeaThreadCv.wait(lock);
        }
    }

    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

void GnssHwTTY::UBX_Thread(void)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    while (!mHelpThreadExit) {
        if (!mUbxBuffer->empty()) {
            UBX_ReaderParse(mUbxBuffer->get());
        } else {
            std::unique_lock<std::mutex> lock(mUbxThreadLock);
            mUbxThreadCv.wait(lock);
        }
    }

    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

int GnssHwTTY::NMEA_Checksum(const char *s)
{
    int crc = 0;

    while (*s) {
        crc ^= *s++;
    }

    return crc;
}

void GnssHwTTY::NMEA_ReaderSplitMessage(std::string msg, std::vector<std::string> & out)
{
    std::string::size_type end = 0;
    bool separator = true;

    out.clear();

    while (!msg.empty()) {
        if ((end = msg.find(",")) == std::string::npos) {
            separator = false;
            end = msg.length();
        }
        out.push_back(std::string(msg.c_str(), end));
        msg.erase(0, end + (separator ? 1 : 0));
    }

    if (separator) {
        out.push_back(std::string(msg.c_str(), msg.length()));
    }
}

void GnssHwTTY::NMEA_ReaderParse(char *msg)
{
    int64_t crc = 0;
    char * p = strstr(msg, "*");

    if (p == nullptr) {/* No CRC field found? Drop message. */
        return;
    }

    crc = strtol(p + 1, nullptr, 16);
    *p = '\0';

    /* Checking for message integrity */
    if (NMEA_Checksum(msg + 1) ^ crc) {
        ALOGW("Drop message due invalid CRC: %s", msg);
        return;
    }

    bool isNMEA = true;
    if (strncmp(msg, "$PUBX", 5) == 0) {
        isNMEA = false;
    }

    if (isNMEA) {
        /* Push RAW NMEA message to system */
        android::hardware::hidl_string nmeaString;
        nmeaString.setToExternal(msg, strlen(msg));

        if (mEnabled) {
            if (mGnssCb != nullptr) {
                auto ret = mGnssCb->gnssNmeaCb(time(nullptr), nmeaString);
                if (!ret.isOk()) {
                    ALOGE("Unable to invoke gnssNmeaCb");
                    ALOGV("[%s, line %d] Unable to invoke gnssNmeaCb", __func__, __LINE__);
                }
            }
        }
    }

    ALOGV("[%s, line %d] GPSRAW: %s", __func__, __LINE__, msg);
    const size_t prefixOffset = 3;
    const size_t pubxPrefixOffset = 6;
    const size_t lenToCmp = 3;
    const size_t pubxLenToCmp = 2;

    /* Parse message */
    if (strncmp(msg + prefixOffset, "RMC", lenToCmp) == 0) {
        NMEA_ReaderParse_GxRMC(msg);
    } else if (strncmp(msg + prefixOffset, "GGA", lenToCmp) == 0) {
        NMEA_ReaderParse_GxGGA(msg);
    } else if (strncmp(msg + prefixOffset, "GSA", lenToCmp) == 0) {
        NMEA_ReaderParse_GxGSA(msg);
    } else if (strncmp(msg + prefixOffset, "GSV", lenToCmp) == 0) {
        NMEA_ReaderParse_xxGSV(msg);
    } else if (isNMEA == false) {
        if (strncmp(msg + pubxPrefixOffset, "00", pubxLenToCmp) == 0) {
            NMEA_ReaderParse_PUBX00(msg);
        } else {
            ALOGD("Unhandled PUBX message");
        }
    } else {
        ALOGV("[%s, line %d] GPSRAW: Unhandled message: %s", __func__, __LINE__, msg);
    }
}

void GnssHwTTY::SetYearOfHardware()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    switch (mUbxGeneration) {
    case ublox7:
        mYearOfHardware = 2012;
        break;
    case ublox8:
        mYearOfHardware = 2016;
        break;
    default:
        mYearOfHardware = 0;
    }
}

uint16_t GnssHwTTY::GetYearOfHardware()
{
    ALOGV("[%s, line %d] YearOfHw %u", __func__, __LINE__, mYearOfHardware);
    return mYearOfHardware;
}

void GnssHwTTY::NMEA_ReaderParse_GxRMC(char *msg)
{
    ALOGV("[%s, line %d] Entry", __func__ ,__LINE__);

    int degree;
    double raw, minutes, polarity;
    struct tm t;

    std::vector<std::string> rmc;
    NMEA_ReaderSplitMessage(std::string(msg), rmc);

    if (rmc.size() != mRmcFieldsNumber || (rmc[0] != "$GPRMC" && rmc[0] != "$GNRMC")) {
        ALOGD("Dropping RMC due to invalid size");
        return;
    }

    // Status A=active or V=Void
    if (rmc[2] != "A") {
        if (mEnabled) {
            ALOGD("GPRMC: No valid fix coordinates, wait...");
        }

        /* Invalidate location data*/
        memset(&mGnssLocation, 0, sizeof(GnssLocation));

        return; /* Only when Active continue parse */
    }

    /* Parse time/date fields */
    memset(&t, 0, sizeof(struct tm));

    // UTC fix time
    if (rmc[1].length() > 0) {
        sscanf(rmc[1].c_str(), "%02u%02u%02u", &t.tm_hour, &t.tm_min, &t.tm_sec);
    }

    // Date
    if (rmc[9].length() > 0) {
        sscanf(rmc[9].c_str(), "%02u%02u%02u", &t.tm_mday, &t.tm_mon, &t.tm_year);

        // fix month (1-12 -> 0-11)
        t.tm_mon -= 1;
        // fix year (1917 -> 2017)
        t.tm_year += 100;
    }

    mGnssLocation.timestamp = mktime(&t) * 1000; // timestamp of the event in milliseconds, mktime(&t) returns seconds, therefore we need to convert

    /* Parse longtitude and latitude */
    mGnssLocation.gnssLocationFlags |= static_cast<uint16_t>(GnssLocationFlags::HAS_LAT_LONG);

    raw = atof(rmc[3].c_str()); // Latitude
    polarity = static_cast<double>(rmc[4] == "S" ? -1 : 1);

    degree = static_cast<int>(raw / 100);
    minutes = ((raw / 100) - degree) * 100;
    mGnssLocation.latitudeDegrees = ((minutes / 60) + degree) * polarity;

    raw = atof(rmc[5].c_str()); // Longitude
    polarity = static_cast<double>(rmc[6] == "W" ? -1.f : 1.f);

    degree = static_cast<int>(raw / 100);
    minutes = ((raw / 100) - degree) * 100;
    mGnssLocation.longitudeDegrees = ((minutes / 60) + degree) * polarity;

    // Speed over the ground in knots
    if (rmc[7].length() > 0) {
        mGnssLocation.speedMetersPerSec = (static_cast<float>(atof(rmc[7].c_str())) * 1.852f) / 3.6f; // knots -> m/s
        mGnssLocation.gnssLocationFlags |= static_cast<uint16_t>(GnssLocationFlags::HAS_SPEED);
        mGnssLocation.speedAccuracyMetersPerSecond = mSpeedAcc;
        mGnssLocation.gnssLocationFlags |= static_cast<uint16_t>(GnssLocationFlags::HAS_SPEED_ACCURACY);
    } else {
        mGnssLocation.speedMetersPerSec = 0.f;
        mGnssLocation.gnssLocationFlags &= ~static_cast<uint16_t>(GnssLocationFlags::HAS_SPEED);
        mGnssLocation.speedAccuracyMetersPerSecond = 0.f;
        mGnssLocation.gnssLocationFlags &= ~static_cast<uint16_t>(GnssLocationFlags::HAS_SPEED_ACCURACY);
    }

    // Track angle in degrees True
    if (rmc[8].length() > 0) {
        mGnssLocation.bearingDegrees = static_cast<float>(atof(rmc[8].c_str()));
        mGnssLocation.gnssLocationFlags |= static_cast<uint16_t>(GnssLocationFlags::HAS_BEARING);
        mGnssLocation.bearingAccuracyDegrees = mBearingAcc;
        mGnssLocation.gnssLocationFlags |= static_cast<uint16_t>(GnssLocationFlags::HAS_BEARING_ACCURACY);
    } else {
        mGnssLocation.bearingDegrees = 0.f;
        mGnssLocation.gnssLocationFlags &= ~static_cast<uint16_t>(GnssLocationFlags::HAS_BEARING);
        mGnssLocation.bearingAccuracyDegrees = 0.f;
        mGnssLocation.gnssLocationFlags &= ~static_cast<uint16_t>(GnssLocationFlags::HAS_BEARING_ACCURACY);
    }

    // For ublox devices location callback depends on speed, bearing and altitude existence
    // Provide location only for 3D fix (has altitude)
    // Provide location only if speed and bearing (course over ground) are provided simultaneously
    bool hasSpeed = (mGnssLocation.gnssLocationFlags & GnssLocationFlags::HAS_SPEED);
    bool hasBearing = (mGnssLocation.gnssLocationFlags & GnssLocationFlags::HAS_BEARING);
    bool hasAltitude = (mGnssLocation.gnssLocationFlags & GnssLocationFlags::HAS_ALTITUDE);
    bool provideLocation = ((!hasSpeed || (hasSpeed && hasBearing)) && hasAltitude);

    mGnssLocation.speedAccuracyMetersPerSecond = mSpeedAcc;
    mGnssLocation.gnssLocationFlags |= static_cast<uint16_t>(GnssLocationFlags::HAS_SPEED_ACCURACY);

    GnssMeasToLocSync& syncInstance = GnssMeasToLocSync::getInstance();
    if (mEnabled && provideLocation && syncInstance.WaitToSend()) {
        ALOGV("[%s, line %d] Provide location callback", __func__, __LINE__);
        if (mGnssCb != nullptr) {
            ALOGD("Provide location callback");
            auto ret = mGnssCb->gnssLocationCb(mGnssLocation);
            if (!ret.isOk()) {
                ALOGE("[%s, line %d]: Unable to invoke gnssLocationCb", __func__, __LINE__);
            }
        }
    }
}

void GnssHwTTY::NMEA_ReaderParse_GxGGA(char *msg)
{
    ALOGV("[%s, line %d] Entry", __func__ ,__LINE__);

    std::vector<std::string> gga;
    NMEA_ReaderSplitMessage(std::string(msg), gga);

    /* Message parts count must be 15 */
    if (gga.size() != ggaFieldsNumber ||
            (gga[0] != "$GPGGA" && gga[0] != "$GNGGA")) {
        ALOGD("Dropping GGA due to invalid size");
        return;
    }

    // Altitude, Meters, above mean sea level
    if (gga[9].length() > 0) {
        mGnssLocation.altitudeMeters = atof(gga[9].c_str());
        mGnssLocation.gnssLocationFlags |= static_cast<uint16_t>(GnssLocationFlags::HAS_ALTITUDE);
    } else {
        mGnssLocation.altitudeMeters = 0;
        mGnssLocation.gnssLocationFlags &= ~static_cast<uint16_t>(GnssLocationFlags::HAS_ALTITUDE);
    }
}

void GnssHwTTY::NMEA_ReaderParse_xxGSV(char *msg)
{
    ALOGV("[%s, line %d] Entry", __func__ ,__LINE__);

    const float L1BandFrequency = 1575.42f;
    const float B1BandFrequency = 1561.098f;
    const float L1GlonassBandFrequency = 1602.562f;
    const float scale = 1000000.0;

    std::vector<std::string> gsv;
    NMEA_ReaderSplitMessage(std::string(msg), gsv);

    /* Message must have least 4 parts */
    if (gsv.size() < gsvFieldsNumber || strncmp(msg+3, "GSV", 3) != 0) {
        ALOGD("Dropping GSV due to invalid size");
        return;
    }

    /*
     * snippet from
     *   u-blox 8 / u-blox M8
     *   Receiver Description
     *
     * GPS, SBAS, QZSS         | GP
     * GLONASS                 | GL
     * Galileo                 | GA
     * BeiDou                  | GB
     * Any combination of GNSS | GN
     */

    SatelliteType currentSatelliteType = SatelliteType::UNKNOWN;
    if (strncmp(msg, "$GP", 3) == 0) {
        currentSatelliteType = SatelliteType::GPS_SBAS_QZSS;
    }
    else if (strncmp(msg, "$GL", 3) == 0) {
        currentSatelliteType = SatelliteType::GLONASS;
    }
    else if (strncmp(msg, "$GA", 3) == 0) {
        currentSatelliteType = SatelliteType::GALILEO;
    }
    else if (strncmp(msg, "$GB", 3) == 0) {
        currentSatelliteType = SatelliteType::BEIDOU;
    }
    else if (strncmp(msg, "$GN", 3) == 0) {
        currentSatelliteType = SatelliteType::ANY;
    }
    else {
        ALOGW("Invalid satellite type received in GSV parser");
        return;
    }

    std::vector<IGnssCallback::GnssSvInfo>* satelliteList = &mSatellites[static_cast<int>(currentSatelliteType)];

    if (gsv.size() % 4) { /* Ommited sattelite c_n0_dbhz param ? */
        gsv.push_back(std::string("0")); /* Append with zero */
    }

    int sentences = std::atoi(gsv[1].c_str());    // total amount of sentences
    int sentence_idx = std::atoi(gsv[2].c_str()); // current sentence
    int num_svs = std::atoi(gsv[3].c_str());      // number of satellites in sentences
    bool valid_msg = true;
    bool callback_ready = false;
    static int glonass_fcn; // GLONASS SVID can be equal to zero, which is not acceptable by Android
    // in this case we are supposed to report FCN (frequency channel number),
    // we don't have a real one, so let's use a simulated one

    if ( sentence_idx <= 0 || sentence_idx > sentences || num_svs <= 0 ) {
        valid_msg = false;
    }

    if (sentences == sentence_idx) {
        callback_ready = true;
    }

    if (sentence_idx == 1) {
        // GPS is always enabled
        // Only one secondary major GNSS can be enabled
        // Make sure that GLONASS messages are not handled when BEIDOU is active and vice versa
        switch (mMajorGnssStatus) {
        case MajorGnssStatus::GPS_GLONASS:
            mSatellites[static_cast<int>(SatelliteType::BEIDOU)].clear();
            mSatellitesUsedInFix[static_cast<int>(SatelliteType::BEIDOU)].clear();
            if (currentSatelliteType == SatelliteType::BEIDOU) {
                return;
            }
            break;

        case MajorGnssStatus::GPS_BEIDOU:
            mSatellites[static_cast<int>(SatelliteType::GLONASS)].clear();
            mSatellitesUsedInFix[static_cast<int>(SatelliteType::GLONASS)].clear();
            if (currentSatelliteType == SatelliteType::GLONASS) {
                return;
            }
            break;

        case MajorGnssStatus::GPS_ONLY:
            mSatellites[static_cast<int>(SatelliteType::BEIDOU)].clear();
            mSatellites[static_cast<int>(SatelliteType::GLONASS)].clear();
            mSatellitesUsedInFix[static_cast<int>(SatelliteType::BEIDOU)].clear();
            mSatellitesUsedInFix[static_cast<int>(SatelliteType::GLONASS)].clear();
            break;
        }

        satelliteList->clear();

        if (currentSatelliteType == SatelliteType::GLONASS) {
            glonass_fcn = 93;
        }
    }

    if (valid_msg) {
        /* Per one GPGSV message max 4 sattelite records */
        size_t offset = static_cast<size_t>((sentence_idx - 1) * 4);
        size_t parts = static_cast<size_t>(std::min((num_svs - static_cast<int>(offset)), 4));

        for (size_t part = 0; part < parts; part++) {
            if ((offset + part) >= static_cast<int>(GnssMax::SVS_COUNT)) {
                break; /* out of space */
            }

            IGnssCallback::GnssSvInfo sv;

            sv.svFlag = static_cast<uint8_t>(IGnssCallback::GnssSvFlags::HAS_ALMANAC_DATA);

            size_t idx = 4 + (part * 4);

            /**
         * Pseudo-random number for the SV, or FCN/OSN number for Glonass. The
         * distinction is made by looking at constellation field. Values must be
         * in the range of:
         *
         * - GNSS:    1-32
         * - SBAS:    120-151, 183-192
         * - GLONASS: 1-24, the orbital slot number (OSN), if known.  Or, if not:
         *            93-106, the frequency channel number (FCN) (-7 to +6) offset by
         *            + 100
         *            i.e. report an FCN of -7 as 93, FCN of 0 as 100, and FCN of +6
         *            as 106.
         * - QZSS:    193-200
         * - Galileo: 1-36
         * - Beidou:  1-37
         */

            sv.svid = static_cast<int16_t>(std::atoi(gsv[idx + 0].c_str()));

            std::vector<int64_t>* usedInFix = &mSatellitesUsedInFix[static_cast<int>(currentSatelliteType)];
            for (auto it = usedInFix->begin(); it != usedInFix->end();) {
                if (*it == sv.svid) {
                    sv.svFlag |= static_cast<uint8_t>(IGnssCallback::GnssSvFlags::USED_IN_FIX);
                    it = usedInFix->erase(it);
                } else {
                    it++;
                }
            }

            if ((sv.svid >= 1 && sv.svid <= 32) && (currentSatelliteType == SatelliteType::GPS_SBAS_QZSS)) {
                sv.constellation = GnssConstellationType::GPS;
                sv.carrierFrequencyHz = (L1BandFrequency * scale);
                sv.svFlag |= static_cast<uint8_t>(IGnssCallback::GnssSvFlags::HAS_CARRIER_FREQUENCY);
            }
            else if ((sv.svid >= 1 && sv.svid <= 36) && (currentSatelliteType == SatelliteType::GALILEO)) {
                sv.constellation = GnssConstellationType::GALILEO;
                sv.carrierFrequencyHz = (L1BandFrequency * scale);
                sv.svFlag |= static_cast<uint8_t>(IGnssCallback::GnssSvFlags::HAS_CARRIER_FREQUENCY);
            }
            else if (currentSatelliteType == SatelliteType::GLONASS) {
                sv.constellation = GnssConstellationType::GLONASS;
                if (sv.svid != 0) {
                    sv.svid -= 64;
                } else {
                    sv.svid = static_cast<int16_t>(glonass_fcn);
                    glonass_fcn++;
                    if (glonass_fcn >= 106) {
                        ALOGW("Failed to generate a fake FCN for GLONASS satellite");
                        glonass_fcn = 0;
                    }
                }
                sv.carrierFrequencyHz = (L1GlonassBandFrequency * scale);
                sv.svFlag |= static_cast<uint8_t>(IGnssCallback::GnssSvFlags::HAS_CARRIER_FREQUENCY);
            }
            else if (currentSatelliteType == SatelliteType::BEIDOU) {
                sv.constellation = GnssConstellationType::BEIDOU;
                sv.carrierFrequencyHz = (B1BandFrequency * scale);
                sv.svFlag |= static_cast<uint8_t>(IGnssCallback::GnssSvFlags::HAS_CARRIER_FREQUENCY);
            }
            else if ((sv.svid >=  33) && (sv.svid <=  64)) {
                sv.constellation = GnssConstellationType::SBAS;
                sv.svid += 87;
                sv.carrierFrequencyHz = (L1BandFrequency * scale);
                sv.svFlag |= static_cast<uint8_t>(IGnssCallback::GnssSvFlags::HAS_CARRIER_FREQUENCY);
            }
            else if ((sv.svid >= 152) && (sv.svid <= 158)) {
                sv.constellation = GnssConstellationType::SBAS;
                sv.svid += 31;
                sv.carrierFrequencyHz = (L1BandFrequency * scale);
                sv.svFlag |= static_cast<uint8_t>(IGnssCallback::GnssSvFlags::HAS_CARRIER_FREQUENCY);
            }
            else if ((sv.svid >= 193) && (sv.svid <= 197)) {
                sv.constellation = GnssConstellationType::QZSS;
                sv.carrierFrequencyHz = (L1BandFrequency * scale);
                sv.svFlag |= static_cast<uint8_t>(IGnssCallback::GnssSvFlags::HAS_CARRIER_FREQUENCY);
            }
            else {
                if (currentSatelliteType != SatelliteType::ANY) {
                    ALOGW("Unknown constellation type with Svid = %d", sv.svid);
                }
                sv.constellation = GnssConstellationType::UNKNOWN;
            }

            sv.elevationDegrees = static_cast<float>(std::atof(gsv[idx + 1].c_str()));
            sv.azimuthDegrees = static_cast<float>(std::atof(gsv[idx + 2].c_str()));
            sv.cN0Dbhz = static_cast<float>(std::atof(gsv[idx + 3].c_str()));

            satelliteList->push_back(sv);

            ALOGV("GPGSV: [%zu] svid=%d, elevation=%f, azimuth=%f, c_n0_dbhz=%f", offset + part,
                  sv.svid, sv.elevationDegrees, sv.azimuthDegrees, sv.cN0Dbhz);
        }
    }

    if (callback_ready) {
        unsigned int svCount = 0;
        for (unsigned int satType = 0; satType < static_cast<int>(SatelliteType::COUNT); satType++) {
            for (std::vector<IGnssCallback::GnssSvInfo>::size_type satNum = 0; satNum < mSatellites[satType].size(); satNum++) {
                mSvStatus.gnssSvList[svCount++] = mSatellites[satType].at(satNum);

                if (svCount >= static_cast<unsigned int>(GnssMax::SVS_COUNT)) {
                    satType = static_cast<int>(SatelliteType::COUNT);
                    satNum = mSatellites[satType].size();
                }
            }
        }
        mSvStatus.numSvs = svCount;

        ALOGV("GPS SV: GPS/SBAS/QZSS: %lu | GLONASS: %lu | GALILEO: %lu | BEIDOU: %lu | UNKNOWN: %lu | total: %u",
              mSatellites[0].size(), mSatellites[1].size(), mSatellites[2].size(), mSatellites[3].size(),
              mSatellites[4].size(), svCount);

        if (sentences == sentence_idx) { /* Last SVS received and parsed */
            if (mEnabled) {
                if (mGnssCb != nullptr) {
                    auto ret = mGnssCb->gnssSvStatusCb(mSvStatus);
                    if (!ret.isOk()) {
                        ALOGE("[%s, line %d]: Unable to invoke gnssSvStatusCb", __func__, __LINE__);
                    }
                }
            }
        }
    }
}

void GnssHwTTY::NMEA_ReaderParse_GxGSA(char* msg)
{
    ALOGV("[%s, line %d] Entry", __func__ ,__LINE__);

    std::vector<std::string> gsa;
    NMEA_ReaderSplitMessage(std::string(msg), gsa);

    /* GSA is expected to have 19 parts */
    if (gsa.size() != mGsaFieldsNumber) {
        ALOGD("Dropping GSA due to invalid size (%zu)", gsa.size());
        return;
    }

    std::vector<int64_t>* satelliteList = nullptr;
    switch (gsa.back().c_str()[0]) {
    case '1':
    case '2':
    case '3':
    case '4':
        satelliteList = &mSatellitesUsedInFix[gsa.back().c_str()[0] - '1'];
        break;
    default:
        ALOGI("Unknown GSA GNSS System ID");
        return;
    }

    satelliteList->clear();
    for (std::vector<std::string>::size_type gsaPart = 3; gsaPart < 15; gsaPart++) {
        if (gsa[gsaPart].length() > 0) {
            satelliteList->push_back(strtol(gsa[gsaPart].c_str(), nullptr, 10));
        } else {
            break;
        }
    }
}

void GnssHwTTY::NMEA_ReaderParse_PUBX00(char *msg)
{
    std::vector<std::string> pubx;
    NMEA_ReaderSplitMessage(std::string(msg), pubx);

    /* PUBX,00 is expected to have 21 parts */
    if ((pubx.size() != pubxFieldsNumber) ||
            (pubx[0] != "$PUBX" && pubx[1] != "00")) {
        ALOGD("Dropping PUBX due to invalid size (%lu)", pubx.size());
        return;
    }

    if (pubx[9].length() > 0) {
        mGnssLocation.horizontalAccuracyMeters = strtof(pubx[9].c_str(), nullptr);
        mGnssLocation.gnssLocationFlags       |= static_cast<uint16_t>(GnssLocationFlags::HAS_HORIZONTAL_ACCURACY);
    } else {
        mGnssLocation.horizontalAccuracyMeters = 0.f;
        mGnssLocation.gnssLocationFlags       &= ~static_cast<uint16_t>(GnssLocationFlags::HAS_HORIZONTAL_ACCURACY);
    }

    if (pubx[10].length() > 0) {
        mGnssLocation.verticalAccuracyMeters = strtof(pubx[10].c_str(), nullptr);
        mGnssLocation.gnssLocationFlags     |= static_cast<uint16_t>(GnssLocationFlags::HAS_VERTICAL_ACCURACY);
    } else {
        mGnssLocation.verticalAccuracyMeters = 0.f;
        mGnssLocation.gnssLocationFlags     &= ~static_cast<uint16_t>(GnssLocationFlags::HAS_VERTICAL_ACCURACY);
    }
}

bool GnssHwTTY::setUpdatePeriod(int periodMs)
{
    requestedUpdateIntervalUs = periodMs * 1000;
    return true;
}

void GnssHwTTY::UBX_Reset()
{
    mUM.state             = UbxState::SYNC1;
    mUM.rx_class          = 0;
    mUM.rx_id             = 0;
    mUM.rx_payload_len    = 0;
    mUM.rx_payload_ptr    = 0;
    mUM.rx_checksum_a     = 0;
    mUM.rx_checksum_b     = 0;
    mUM.rx_exp_checksum_a = 0;
    mUM.rx_exp_checksum_b = 0;
    mUM.buffer_ptr        = 0;
    mUM.rx_timedout       = false;
    mUM.msg_payload       = nullptr;
}

void GnssHwTTY::UBX_ChecksumAdd(uint8_t ch)
{
    mUM.rx_checksum_a = static_cast<uint8_t>(mUM.rx_checksum_a + ch);
    mUM.rx_checksum_b = static_cast<uint8_t>(mUM.rx_checksum_b + mUM.rx_checksum_a);
}

void GnssHwTTY::UBX_CriticalProtocolError(const char *errormsg)
{
    ALOGE("UBX Critical protocol error: %s", errormsg);
    CHECK_EQ(0, 1) << "UBX Critical protocol error: " << errormsg;
}

void GnssHwTTY::UBX_ReaderParse(UbxBufferElement *ubx)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    for (mUM.buffer_ptr = 0; mUM.buffer_ptr < ubx->len; mUM.buffer_ptr++) {
        ALOGV("UBX parser state: %d (buffer ptr: %lu)", mUM.state, mUM.buffer_ptr);
        switch (mUM.state) {
        case UbxState::SYNC1:
            ALOGV("[%s, line %d] Sync1", __func__, __LINE__);
            if (ubx->data[mUM.buffer_ptr] == mUbxSync1) {
                mUM.state = UbxState::SYNC2;
            } else {
                ALOGW("UBX parser error on state UbxState::SYNC1 (%02X)", ubx->data[mUM.buffer_ptr]);
            }
            break;

        case UbxState::SYNC2:
            ALOGV("[%s, line %d] Sync2", __func__, __LINE__);
            if (ubx->data[mUM.buffer_ptr] == mUbxSync2) {
                mUM.state = UbxState::CLASS;
            } else {
                ALOGW("UBX parser error on state UbxState::SYNC2 (%02X)", ubx->data[mUM.buffer_ptr]);
            }
            break;

        case UbxState::CLASS:
            ALOGV("[%s, line %d] Class", __func__, __LINE__);
            UBX_ChecksumAdd(ubx->data[mUM.buffer_ptr]);
            mUM.rx_class = ubx->data[mUM.buffer_ptr];
            mUM.state    = UbxState::ID;
            break;

        case UbxState::ID:
            ALOGV("[%s, line %d] Id", __func__, __LINE__);
            UBX_ChecksumAdd(ubx->data[mUM.buffer_ptr]);
            mUM.rx_id = ubx->data[mUM.buffer_ptr];
            mUM.state = UbxState::LENGTH1;
            break;

        case UbxState::LENGTH1:
            ALOGV("[%s, line %d] Length1", __func__, __LINE__);
            UBX_ChecksumAdd(ubx->data[mUM.buffer_ptr]);
            mUM.rx_payload_len = ubx->data[mUM.buffer_ptr];
            mUM.state          = UbxState::LENGTH2;
            break;

        case UbxState::LENGTH2:
            ALOGV("[%s, line %d] Length2", __func__, __LINE__);
            UBX_ChecksumAdd(ubx->data[mUM.buffer_ptr]);
            mUM.rx_payload_len += ubx->data[mUM.buffer_ptr] << 8;
            mUM.state           = UbxState::PAYLOAD;
            ALOGV("UBX payload len: %d", mUM.rx_payload_len);
            if (mUM.rx_payload_len == 0) {
                mUM.state = UbxState::CHECKSUM1;
            }
            break;

        case UbxState::PAYLOAD:
            ALOGV("[%s, line %d] Payload", __func__, __LINE__);
            UBX_ChecksumAdd(ubx->data[mUM.buffer_ptr]);
            if (nullptr == mUM.msg_payload) {
                mUM.msg_payload = &(ubx->data[mUM.buffer_ptr]);
            }

            mUM.rx_payload_ptr++;
            if (mUM.rx_payload_ptr == mUM.rx_payload_len) {
                mUM.state = UbxState::CHECKSUM1;
            }
            break;

        case UbxState::CHECKSUM1:
            ALOGV("[%s, line %d] crc 1", __func__, __LINE__);
            mUM.rx_exp_checksum_a = ubx->data[mUM.buffer_ptr];
            mUM.state = UbxState::CHECKSUM2;
            break;

        case UbxState::CHECKSUM2:
            ALOGV("[%s, line %d] crc 2", __func__, __LINE__);
            mUM.rx_exp_checksum_b = ubx->data[mUM.buffer_ptr];
            mUM.state = UbxState::FINISH;
            break;

        case UbxState::FINISH:
            ALOGW("UBX Unexpected FINISH state in parser");
            break;
        }
    }

    if (mUM.state == UbxState::FINISH) {
        ALOGV("[%s, line %d] GnssUbx received message CLASS: %02X ID: %02X, len = %u", __func__, __LINE__, mUM.rx_class, mUM.rx_id, mUM.rx_payload_len);

        if (mUM.rx_exp_checksum_a != mUM.rx_checksum_a ||
                mUM.rx_exp_checksum_b != mUM.rx_checksum_b) {
            ALOGI("[%s, line %d] UBX parser checksum fail %02X%02X/%02X%02X", __func__, __LINE__, mUM.rx_exp_checksum_a, mUM.rx_exp_checksum_b,
                  mUM.rx_checksum_a, mUM.rx_checksum_b);
        } else {
            ALOGV("[%s, line %d] UBX parser checksum ok", __func__, __LINE__);
        }

        SelectParser(mUM.rx_class, mUM.rx_id, mUM.msg_payload, mUM.rx_payload_len);

    } else {
        ALOGI("[%s, line %d] UBX message incomplete, dropping (state: %d, buf: %zu/%zu, buf[ptr]: %02X",__func__, __LINE__, static_cast<int>(mUM.state),
              mUM.buffer_ptr, ubx->len, ubx->data[mUM.buffer_ptr]);
        for (size_t i = 0; i < ubx->len; i++) {
            ALOGD("[%s, line %d] UBX payload[%zu] = %02X", __func__, __LINE__, i, ubx->data[i]);
        }
    }

    UBX_Reset();
}

void GnssHwTTY::UBX_Send(const uint8_t* msg, size_t len)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (nullptr == msg) {
        ALOGD("[%s, line %d] Null input msg", __func__, __LINE__);
        ALOGE("Null msg to send");
        return;
    }

    if (len >= mUbxBufferSize) {
        len = mUbxBufferSize - 1;
    }

    uint8_t tx_buffer[mUbxBufferSize];

    // header (sync)
    tx_buffer[0] = mUbxSync1;
    tx_buffer[1] = mUbxSync2;

    // data
    memcpy(&tx_buffer[2], msg, len);

    // checksum
    size_t checksum_offset_a = 2 + len;
    size_t checksum_offset_b = checksum_offset_a + 1;

    tx_buffer[checksum_offset_a] = 0;
    tx_buffer[checksum_offset_b] = 0;

    for (size_t i = 0; i < len; i++) {
        tx_buffer[checksum_offset_a] = static_cast<uint8_t>(tx_buffer[checksum_offset_a] + msg[i]);
        tx_buffer[checksum_offset_b] = static_cast<uint8_t>(tx_buffer[checksum_offset_b] + tx_buffer[checksum_offset_a]);
    }

    // class, id
    mUM.tx_class = msg[0];
    mUM.tx_id    = msg[1];

    ALOGV("UBX sending msg...");
    size_t exp_len = len + 4;

    ssize_t ret = write(mFd, tx_buffer, exp_len);
    if (ret < static_cast<ssize_t>(exp_len)) {
        ALOGE("UBX send msg: failed to transmit fully (only %ld of %zu bytes trasmitted)\n", ret, exp_len);
    } else if (ret == static_cast<ssize_t>(-1)) {
        ALOGE("UBX send msg: failed to write message, write error: %s\n", strerror(errno));
    } else {
        ALOGV("[%s, line %d] UBX sent msg: CLASS: %02X ID: %02X CHECKSUM: %02X%02X", __func__, __LINE__,
              tx_buffer[2], tx_buffer[3], tx_buffer[checksum_offset_a], tx_buffer[checksum_offset_b]);
    }
}

void GnssHwTTY::UBX_Expect(UbxRxState astate, const char* errormsg)
{
    UbxStateQueueElement element;
    element.state    = astate;
    element.xclass   = mUM.tx_class;
    element.id       = mUM.tx_id;
    if (errormsg != nullptr) {
        element.errormsg = errormsg;
    } else {
        element.errormsg = "UBX protocol error (hardware response timeout)";
    }
    mUbxStateBuffer->put(&element);
}

bool GnssHwTTY::UBX_Wait(UbxRxState astate, const char* errormsg, int64_t timeoutMs)
{
    UBX_Expect(astate, errormsg);

    timeoutMs *= 1024 * 1024; // mili to micro to nano
    int64_t start = android::elapsedRealtimeNano();
    while (mUbxAckReceived <= 0) {
        if ((android::elapsedRealtimeNano() - start) > timeoutMs) {
            if (errormsg != nullptr) {
                UBX_CriticalProtocolError(errormsg);
            } else {
                ALOGE("[%s, line %d] UBX protocol failure (no ACK received)", __func__, __LINE__);
                UBX_CriticalProtocolError("UBX protocol failure (no ACK received)");
            }
            return false;
        }
        ALOGV("UBX Wait...");
        usleep(25000); // sleep for 25 ms
        mNmeaThreadCv.notify_all();
        mUbxThreadCv.notify_all();
    }

    mUbxAckReceived--;
    return true;
}

void GnssHwTTY::UBX_SetMessageRate(uint8_t msg_class, uint8_t msg_id, uint8_t rate, const char *msg)
{
    uint8_t ublox_buf[] = {0x06, 0x01, 0x08, 0x00, msg_class, msg_id,
                           rate, rate, 0x00, rate, rate, 0x00};
    UBX_Send(ublox_buf, sizeof(ublox_buf));
    UBX_Wait(UbxRxState::WAITING_ACK, msg, mUbxTimeoutMs);
}

void GnssHwTTY::UBX_SetMessageRateCurrentPort(uint8_t msg_class, uint8_t msg_id, uint8_t rate, const char* msg)
{
    uint8_t ublox_buf[] = {0x06, 0x01, 0x03, 0x00, msg_class, msg_id, rate};
    UBX_Send(ublox_buf, sizeof(ublox_buf));
    UBX_Wait(UbxRxState::WAITING_ACK, msg, mUbxTimeoutMs);
}

bool GnssHwTTY::CheckHwPropertyKf()
{
    char prop_hardware[PROPERTY_VALUE_MAX] = {};
    std::string kingfisher("kingfisher");
    std::string propHardware("ro.hardware");

    int result = __system_property_get(propHardware.c_str(), prop_hardware);
    if (result > 0) {
        if (0 == kingfisher.compare(prop_hardware)) {
            mIsKingfisher = true;
            mUbxGeneration = ublox8;
            SetYearOfHardware();
            return true;
        }
    }
    return false;
}

bool GnssHwTTY::CheckUsbDeviceVendorUbx()
{
    UsbHandler usbHandler;
    return usbHandler.ScanUsbDevices(mUbxGeneration);
}

void GnssHwTTY::SelectParser(uint8_t cl, uint8_t id, const uint8_t* data, uint16_t dataLen)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    if (nullptr == data) {
        ALOGV("[%s, line %d] Wrong input", __func__, __LINE__);
        return;
    }

    GnssMeasQueue& instance = GnssMeasQueue::getInstance();

    if (cl == classUbxRxm && id == idMeasx) {
        auto sp = std::make_shared<GnssRxmMeasxParser>(data, dataLen);
        instance.push(sp);
    } else if (cl == classUbxNav && id == idClock) {
        auto sp = std::make_shared<GnssNavClockParser>(data, dataLen);
        instance.push(sp);
    } else if (cl == classUbxNav && id == idTimeGps) {
        auto sp = std::make_shared<GnssNavTimeGPSParser>(data, dataLen);
        instance.push(sp);
    } else if (cl == classUbxNav && id == idStatus) {
        auto sp = std::make_shared<GnssNavStatusParser>(data, dataLen);
        instance.push(sp);
    } else if (mAckClass == cl && mAckAckId == id) {
        UBX_ACKParse(data, dataLen);
    } else if (mAckClass == cl && mAckNakId == id) {
        UBX_NACKParse(data, dataLen);
    }

    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
}

void GnssHwTTY::UBX_ACKParse(const uint8_t* data, uint16_t dataLen)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (nullptr == data || ackNackMsgLen != dataLen) {
        ALOGV("[%s, line %d] Wrong input", __func__, __LINE__);
        return;
    }

    UbxStateQueueElement *st = nullptr;
    if (!mUbxStateBuffer->empty()) {
        st = mUbxStateBuffer->get();
        if (UbxRxState::WAITING_ACK == st->state) {
            if (st->xclass == data[ackNackClassOffset] && st->id == data[ackNackIdOffset]) {
                mUbxAckReceived++;
            } else {
                UBX_CriticalProtocolError(st->errormsg);
            }
        }
    }
}

void GnssHwTTY::UBX_NACKParse(const uint8_t* data, uint16_t dataLen)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);

    if (nullptr == data || ackNackMsgLen != dataLen) {
        ALOGV("[%s, line %d] Wrong input", __func__, __LINE__);
        return;
    }

    UbxStateQueueElement *st = nullptr;
    if (!mUbxStateBuffer->empty()) {
        st = mUbxStateBuffer->get();
        if (UbxRxState::WAITING_ACK == st->state) {
            if (st->xclass == data[ackNackClassOffset] && st->id == data[ackNackIdOffset]) {
                UBX_CriticalProtocolError(st->errormsg);
            }
        }
    }
}
