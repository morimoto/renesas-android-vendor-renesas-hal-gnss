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

#include "include/GnssTransport.h"

#include <log/log.h>

namespace android::hardware::gnss::V2_0::renesas {

TError Transport::SetUp() {
    ALOGV("%s", __func__);
    return TError::TransportReady;
}

TError Transport::Reset() {
    ALOGV("%s", __func__);
    mState = Close();
    return Open();
}

TError Transport::GetTransportState() {
    ALOGV("%s", __func__);
    return mState;
}

Transport::Transport(const std::string &filePath):
    mFilePath(filePath) {
    ALOGV("%s", __func__);
    SetEndianness();
}

Transport::~Transport() {
    ALOGV("%s", __func__);
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

std::string Transport::GetPath() const {
    return mFilePath;
}

} // namespace android::hardware::gnss::V2_0::renesas
