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

using android::sp;
using namespace android::hardware::gnss::V1_0;

class GnssHwIface : public android::RefBase
{
public:
    GnssHwIface(void) :
        mThreadExit(false),
        mThread { &GnssHwIface::GnssHwHandleThread, this }
    { }

    virtual ~GnssHwIface(void)
    {
        mThreadExit = true;
        if (mThread.joinable()) mThread.join();
    }

    virtual bool start(void) = 0;
    virtual bool stop(void) = 0;
    virtual void GnssHwHandleThread(void) = 0;

    void setCallback(const sp<::IGnssCallback>& callback) {
        mGnssCb = callback;
    }

    std::atomic<bool>               mThreadExit;
    sp<IGnssCallback>               mGnssCb = nullptr;

    GnssLocation                    mGnssLocation;
    IGnssCallback::GnssSvStatus     mSvStatus;

private:
    std::thread         mThread;
};

class GnssHwTTY : public GnssHwIface
{
    int     mFd;
    char    mNmeaReaderBuf[512];
    size_t  mNmeaReaderBufPos = 0;
    bool    mNmeaReaderInCapture = false;

    int NMEA_Checksum(const char *s);
    void NMEA_ReaderSplitMessage(std::string msg, std::vector<std::string> &out);
    void NMEA_ReaderPushChar(char ch);

    void NMEA_ReaderParse(char *msg);
    void NMEA_ReaderParse_GxRMC(char *msg);
    void NMEA_ReaderParse_GxGGA(char *msg);
    void NMEA_ReaderParse_GPGSV(char *msg);

public:
    GnssHwTTY(void);
    virtual ~GnssHwTTY(void);

    bool start(void);
    bool stop(void);

    void GnssHwHandleThread(void);
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
    virtual ~GnssHwFAKE(void);

    bool start(void);
    bool stop(void);

    void GnssHwHandleThread(void);
};


#endif /* GNSS_HW_H_ */
