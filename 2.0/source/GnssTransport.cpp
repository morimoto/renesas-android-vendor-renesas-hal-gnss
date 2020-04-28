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

#define LOG_NDEBUG 1
#define LOG_TAG "GnssRenesasHalTransport"

#include <log/log.h>
#include <termios.h>
#include <fcntl.h>
#include "include/GnssTransport.h"

char Transport::ReadByte(TError& errCode) {
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

TError Transport::Open(GnssReceiverType recType) {
    ALOGV("%s", __func__);

    if (GnssReceiverType::FakeReceiver == recType) {
        return OpenFakeFile();
    }

    return OpenTTY();
}

TError Transport::OpenFakeFile() {
    return TError::TransportReady;
}

TError Transport::OpenTTY() {
    ALOGV("%s", __func__);
    std::lock_guard<std::mutex> lock(readWriteLock);
    ALOGV("%s, %s", __func__, mPath.c_str());

    do {
        mFd = ::open(mPath.c_str(), O_RDWR | O_NOCTTY);
    } while (mFd <= closedFd && errno == EINTR);

    if (closedFd >= mFd) {
        ALOGE("Could not open TTY device");
        return TError::TransportNotReady;
    }

    return SetUpLine();
}

TError Transport::Close() {
    ALOGV("%s", __func__);
    std::lock_guard<std::mutex> lock(readWriteLock);

    if (closedFd != mFd) {
        ::close(mFd);
        mFd = closedFd;
    }

    return TError::TransportNotReady;
}

TError Transport::SetUpLine() {
    // Setup serial port
    ALOGV("%s", __func__);
    struct termios  ios;
    ::tcgetattr(mFd, &ios);
    ios.c_cflag  = CS8 | CLOCAL | CREAD;
    ios.c_iflag  = IGNPAR;
    ios.c_oflag  = 0;
    ios.c_lflag  = 0;  // disable ECHO, ICANON, etc...

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

    default: {
        ios.c_cflag |= B9600;
        ALOGW("Unsupported baud rate %d.. setting default 9600", mBaudRate);
    }
    }

    ::tcsetattr(mFd, TCSANOW, &ios);
    ::tcflush(mFd, TCIOFLUSH);
    return TError::TransportReady;
}

TError Transport::ResetTransport(const std::shared_ptr<IGnssReceiver>&
                                 receiver) {
    ALOGV("%s", __func__);
    Close();

    if (nullptr == receiver) {
        mPath.clear();
        mBaudRate = badBaudRate;
        ALOGW("Failed to reset transport with null receiver");
        return TError::TransportNotReady;
    } else {
        receiver->GetPath(mPath);
        mBaudRate = receiver->GetBaudRate();
    }

    return Open(receiver->GetReceiverType());
}

TError Transport::GetTransportState() {
    ALOGV("%s", __func__);
    return mState;
}

Transport::Transport(const std::shared_ptr<IGnssReceiver>& receiver) {
    ALOGV("%s", __func__);
    SetEndianness();

    if (TError::TransportReady != ResetTransport(receiver)) {
        ALOGE("Transport not valid");
    }
}

Transport::~Transport() {
    ALOGV("%s", __func__);
    Close();
}

TError Transport::SetEndianness() {
    ALOGV("%s", __func__);
    union {
        uint32_t dec;
        char literal[4];
    } checker = {0x01020304};

    if (0x01 == checker.literal[0]) {
        mEndianType = Endian::Big;
    } else {
        mEndianType = Endian::Little;
    }

    return TError::Success;
}

Endian Transport::GetEndianType() const {
    return mEndianType;
}
