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

#define LOG_TAG "GnssRenesasHalTransportTTY"
#define LOG_NDEBUG 1

#include "include/GnssTransportTTY.h"

#include <libgpio.h>
#include <log/log.h>
#include <termios.h>
#include <unistd.h>

namespace android::hardware::gnss::V2_1::renesas {

static constexpr uint32_t badBaudRate = 0;
static constexpr int closedFd = -1;

GnssTransportTTY::GnssTransportTTY(const std::string &filePath):
    Transport(filePath), mBaudRate(badBaudRate), mFd(closedFd) {
}

GnssTransportTTY::~GnssTransportTTY() {
    if (closedFd != mFd) {
        Close();
    }
}

TError GnssTransportTTY::SetBaudRate(const uint32_t& baudrate) {
    TError status = TError::TransportReady;
    uint32_t oldBaudRate = mBaudRate;
    mBaudRate = baudrate;

    if (mFd > closedFd && baudrate != oldBaudRate) {
        status = SetUp();
    }

    if (TError::TransportReady != status) {
        mBaudRate = oldBaudRate;
    }

    return status;
}

TError GnssTransportTTY::SetUp() {
    // Setup serial port
    ALOGI("%s, baudrate: %u for fd %d (%s)", __func__, mBaudRate, mFd,
          GetPath().c_str());
    struct termios ios;

    if (tcdrain(mFd)) {
        ALOGW("tcdrain has failed! This is not fatal, but can result in "
              "further errors\n");
    }

    ::tcgetattr(mFd, &ios);
    ios.c_cflag = CS8 | CLOCAL | CREAD;
    ios.c_iflag = IGNPAR;
    ios.c_oflag = 0;
    ios.c_lflag = 0;  // disable ECHO, ICANON, etc...

    // Set baudrate
    switch (mBaudRate) {
        case 2400: {
            ios.c_cflag |= B2400;
            break;
        }

        case 4800: {
            ios.c_cflag |= B4800;
            break;
        }

        case 9600: {
            ios.c_cflag |= B9600;
            break;
        }

        case 19200: {
            ios.c_cflag |= B19200;
            break;
        }

        case 38400: {
            ios.c_cflag |= B38400;
            break;
        }

        case 57600: {
            ios.c_cflag |= B57600;
            break;
        }

        case 115200: {
            ios.c_cflag |= B115200;
            break;
        }

        default: {
            ios.c_cflag |= B9600;
            ALOGW("Unsupported baud rate %d.. setting default 9600", mBaudRate);
        }
    }

    if (::tcsetattr(mFd, TCSANOW, &ios)) {
        return TError::TransportNotReady;
    }

    return Transport::SetUp();
}

TError GnssTransportTTY::Open() {
    std::lock_guard<std::mutex> lock(readLock);
    ALOGV("%s, %s", __func__, GetPath().c_str());
    TError res = TError::FailedToRead;

    /* In case of some boards, we may change the default baud rate,
     * so when HAL is restarted without Power off, we need to
     * reset device to make sure it operates at tty_default_rate
     */
    if (mAllowReset) {
        res = ResetReceiver();

        if (TError::Success != res) {
            ALOGW("Can not reset GNSS device, this may cause further malfunction!\n");
        }
    }

    do {
        mFd = ::open(GetPath().c_str(), O_RDWR | O_NOCTTY);
    } while (mFd <= closedFd && errno == EINTR);

    if (closedFd >= mFd) {
        ALOGE("Could not open TTY device %s - %s", GetPath().c_str(),
              strerror(errno));
        return TError::TransportNotReady;
    }

    res = SetUp();

    if (res != TError::TransportReady) {
        return res;
    }

    return TError::TransportReady;
}

TError GnssTransportTTY::Close() {
    ALOGV("%s", __func__);
    std::scoped_lock lock(readLock, writeLock);

    if (closedFd != mFd) {
        ::close(mFd);
        mFd = closedFd;
    }

    return Transport::Close();
}

TError GnssTransportTTY::WriteData(const std::vector<uint8_t> &toWrite) {
    if (closedFd < mFd) {
        ALOGV("%s, going to write", __func__);
        ssize_t ret = ::write(mFd, toWrite.data(), toWrite.size());

        if (ret == static_cast<ssize_t>(toWrite.size())) {
            ALOGV("%s, success", __func__);
            return TError::Success;
        }
    }

    ALOGE("%s, failed_to_write", __func__);
    return TError::FailedToWrite;
}

char GnssTransportTTY::ReadByte(TError& errCode) {
    char byte = 0;
    errCode = TError::FailedToRead;

    if (closedFd < mFd) {
        ssize_t ret = ::read(mFd, &byte, sizeof(byte));

        if (ret == static_cast<ssize_t>(sizeof(byte))) {
            errCode = TError::Success;
        }
    }

    return byte;
}

TError GnssTransportTTY::ResetReceiver() {
    const int delayOff = 200000;
    const int delayOn = 1000000;
    int ret = libgpio_set_value(GPS_RST_CHIP, GPS_RST_LINE, OFF);

    if (!ret) {
        usleep(delayOff);
        ret = libgpio_set_value(GPS_RST_CHIP, GPS_RST_LINE, ON);

        if (!ret) {
            usleep(delayOn);
        }
    }

    return ret < 0 ? TError::InternalError : TError::Success;
}

} // namespace android::hardware::gnss::V2_1::renesas
