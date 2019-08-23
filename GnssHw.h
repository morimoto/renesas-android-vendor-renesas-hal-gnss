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

#ifndef GNSS_HW_H_
#define GNSS_HW_H_

#include <utils/RefBase.h>
#include <thread>
#include <vector>
#include <string>
#include <cstdint>
#include <chrono>
#include <condition_variable>
#include <log/log.h>

#include "circular_buffer.h"
#include <android/hardware/gnss/1.0/IGnss.h>

using namespace std::chrono_literals;

using android::sp;
using namespace android::hardware::gnss::V1_0;

class GnssHwIface : public android::RefBase
{
protected:
    GnssHwIface() {}
    int requestedUpdateIntervalUs;

public:
    void SetUpHandleThread()
    {
        mThreadExit = false;
        mThread = std::thread(&GnssHwIface::GnssHwHandleThread, this);
    }

    virtual ~GnssHwIface();

    virtual bool start(void) = 0;
    virtual bool stop(void) = 0;
    virtual void GnssHwHandleThread(void) = 0;
    virtual bool setUpdatePeriod(int) = 0;
    virtual uint16_t GetYearOfHardware() = 0;

    void setCallback(const sp<::IGnssCallback>& callback) {
        mGnssCb = callback;
    }

    std::atomic<bool>               mThreadExit;
    sp<IGnssCallback>               mGnssCb = nullptr;

    GnssLocation                    mGnssLocation;
    IGnssCallback::GnssSvStatus     mSvStatus;

    bool                            mResetReceiverOnStart = false;

private:
    std::thread         mThread;
};

class GnssHwTTY : public GnssHwIface
{
    enum class ReaderState {
        WAITING,
        CAPTURING_NMEA,
        WAITING_UBX_SYNC2,
        CAPTURING_UBX
    };

    static const size_t mNmeaBufferSize = 128;
    static const size_t mUbxBufferSize  = 65536;

    int          mFd;
    bool         mEnabled;
    uint8_t      mReaderBuf[mUbxBufferSize];
    size_t       mReaderBufPos = 0;
    ReaderState  mReaderState  = ReaderState::WAITING;

    bool mIsKingfisher = false;
    bool mIsUbloxDevice = false;
    uint16_t mUbxGeneration = 0;

    size_t mRmcFieldsNumber;
    size_t mGsaFieldsNumber;

    float mBearingAcc;
    float mSpeedAcc;

    uint16_t mYearOfHardware = 0; // for under 2016 year zero is legal value.

    struct NmeaBufferElement {
        char data[mNmeaBufferSize];
    };

    struct UbxBufferElement {
        uint8_t data[mUbxBufferSize];
        size_t  len;
    };

    CircularBuffer<NmeaBufferElement> *mNmeaBuffer;
    CircularBuffer<UbxBufferElement>  *mUbxBuffer;

    std::thread mNmeaThread;
    std::thread mUbxThread;
    std::thread mHwInitThread;

    std::atomic<bool> mHelpThreadExit;

    std::condition_variable mNmeaThreadCv;
    std::condition_variable mUbxThreadCv;
    std::condition_variable mHandleThreadCv;

    std::mutex mNmeaThreadLock;
    std::mutex mUbxThreadLock;
    std::mutex mHandleThreadLock;

    double mUbxFirmwareVersion = 0.0;
    std::atomic<bool> mFwVersionReady = false;
    std::condition_variable mFirmwareCv;
    std::mutex mFwLock;

    enum class SatelliteType {
        GPS_SBAS_QZSS = 0,
        GLONASS       = 1,
        GALILEO       = 2,
        BEIDOU        = 3,
        ANY           = 4,
        COUNT         = 5,
        UNKNOWN       = 6
    };
    std::vector<IGnssCallback::GnssSvInfo> mSatellites[static_cast<int>(SatelliteType::COUNT)];

    std::vector<int64_t> mSatellitesUsedInFix[static_cast<int>(SatelliteType::COUNT)];

    enum class MajorGnssStatus {
        GPS_GLONASS,
        GPS_BEIDOU,
        GPS_ONLY
    };

    MajorGnssStatus mMajorGnssStatus;

    bool CheckUsbDeviceVendorUbx();
    bool CheckHwPropertyKf();
    void resetOnStart();

    void ReaderPushChar(unsigned char ch);

    void NMEA_Thread(void);
    int  NMEA_Checksum(const char* s);
    void NMEA_ReaderSplitMessage(std::string msg, std::vector<std::string> &out);

    void NMEA_ReaderParse(char* msg);
    void NMEA_ReaderParse_GxRMC(char* msg);
    void NMEA_ReaderParse_GxGGA(char* msg);
    void NMEA_ReaderParse_xxGSV(char* msg);
    void NMEA_ReaderParse_GxGSA(char* msg);
    void NMEA_ReaderParse_PUBX00(char* msg);

    enum class UbxState {
        SYNC1,
        SYNC2,
        CLASS,
        ID,
        LENGTH1,
        LENGTH2,
        PAYLOAD,
        CHECKSUM1,
        CHECKSUM2,
        FINISH
    };

    const uint8_t mAckClass = 0x05;
    const uint8_t mAckAckId = 0x01;
    const uint8_t mAckNakId = 0x00;

    const uint8_t mUbxSync1               = 0xB5;
    const uint8_t mUbxSync2               = 0x62;
    const size_t  mUbxLengthFirstByteNo   = 4;
    const size_t  mUbxLengthSecondByteNo  = 5;
    const size_t  mUbxPacketSizeNoPayload = 8;
    const size_t  mUbxFirstPayloadOffset  = 6;

    const int64_t mUbxTimeoutMs = 5000;

    std::atomic<int> mUbxAckReceived;

    enum class UbxRxState {
        NORMAL,
        WAITING_ANSWER,
        WAITING_ACK
    };

    struct UbxMachine {
        UbxState    state;
        uint8_t     rx_class;
        uint8_t     rx_id;
        uint16_t    rx_payload_len;
        uint16_t    rx_payload_ptr;
        uint8_t     rx_checksum_a; // checksum that we get in the message itself
        uint8_t     rx_checksum_b; // checksum that we get in the message itself
        uint8_t     rx_exp_checksum_a; // checksum that we calculate ourselfs (expected checksum) to test the received data checksum (first part)
        uint8_t     rx_exp_checksum_b; // checksum that we calculate ourselfs (expected checksum) to test the received data checksum (second part)

        uint8_t     tx_class;
        uint8_t     tx_id;

        size_t      buffer_ptr;

        bool        rx_timedout;
        uint8_t*    msg_payload;
    } mUM;

    struct UbxStateQueueElement {
        UbxRxState state;
        uint8_t xclass;
        uint8_t id;
        const char* errormsg;
    };

    CircularBuffer<UbxStateQueueElement> *mUbxStateBuffer;

protected:
    bool OpenDevice(const char* ttyDevDefault);
    bool StartSalvatorProcedure();

    void SelectParser(uint8_t cl, uint8_t id, const uint8_t* data, uint16_t dataLen);
    void RunWorkerThreads();
    void JoinWorkerThreads();

    void ClearConfig();
    void ConfigGnssUblox7();
    void ConfigGnssUblox8();
    void SetNMEA23();
    void SetNMEA41();
    void PollCommonMessages();
    void PollMonVer();
    void PrepareGnssConfig(char* propSecmajor, char* propSbas, uint8_t* ubxCfgGnss, const size_t& cfgSize);
    void SetConstValuesOfHardware(uint16_t gen);

    void InitUblox7Gen();
    void InitUblox8Gen();

    void GnssHwUbxInitThread(void);
    void UBX_Thread(void);
    void UBX_Reset();
    void UBX_ChecksumReset();
    void UBX_ChecksumAdd(uint8_t ch);
    void UBX_ReaderParse(UbxBufferElement* ubx);
    void UBX_Send(const uint8_t* msg, size_t len);
    void UBX_Expect(UbxRxState astate, const char* errormsg); // Expect state (non blocking)
    bool UBX_Wait(UbxRxState astate, const char* errormsg, int64_t timeoutMs);   // Wait state (blocking)
    void UBX_CriticalProtocolError(const char* errormsg);

    void UBX_SetMessageRate(uint8_t msg_class, uint8_t msg_id, uint8_t rate, const char* msg);
    void UBX_SetMessageRateCurrentPort(uint8_t msg_class, uint8_t msg_id, uint8_t rate, const char* msg);

    void SetYearOfHardware();
    void UBX_ACKParse(const uint8_t* data, uint16_t dataLen);
    void UBX_NACKParse(const uint8_t* data, uint16_t dataLen);
    void UBX_MonVerParse(const char* data, uint16_t dataLen);
    GnssHwTTY();

public:
    GnssHwTTY(int fd);
    virtual ~GnssHwTTY(void) override;

    bool start(void) override;
    bool stop(void) override;
    bool setUpdatePeriod(int) override;

    uint16_t GetYearOfHardware() override;
    void GnssHwHandleThread(void) final;
};

class GnssHwFAKE : public GnssHwIface
{
    struct FakeLocationPoint {
        double latitude;
        double longitude;
        float  speed;
    };

    std::vector<FakeLocationPoint> mFakePoints;

    void SplitLine(std::string line, std::vector<std::string> &out);
public:
    GnssHwFAKE(void);
    ~GnssHwFAKE(void) override;

    bool start(void) override;
    bool stop(void) override;
    bool setUpdatePeriod(int) override;

    void GnssHwHandleThread(void) override;
    uint16_t GetYearOfHardware() override {return 0;}
};


#endif /* GNSS_HW_H_ */
