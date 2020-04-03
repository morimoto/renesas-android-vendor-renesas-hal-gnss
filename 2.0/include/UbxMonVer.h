/*
 * Copyright (C) 2019 GlobalLogic
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

#include <vector>
#include <string>
#include <cmath>
#include <regex>
#include <algorithm>
#include <log/log.h>

#include "include/UbxParserCommon.h"

template <typename ClassType>
class UbxMonVer : public UbxParserCommon<ClassType> {
public:
    UbxMonVer(const char* in, const size_t& inLen);
    ~UbxMonVer() override;

    UbxMsg GetMsgType() override;
    UPError GetData(ClassType out) override;
    bool IsValid() override;

protected:
    UbxMonVer();
    UPError Parse();
    UPError ParseSingleBlock();
    UPError ParseRepeatedBlocks();
    UPError ValidateParcel();

private:
    enum MonVerOffsets : uint8_t {
        SwVerOffset = 0,
        HwVerOffset = 30,
        ExtensionsOffset = 40,
    };

    typedef struct SingleBlock {
        double swVersion;
        size_t hwVersion;
    } singleBlock_t;

    const char* mPayload;
    const size_t mPayloadLen;

    const UbxMsg mType = UbxMsg::MON_VER;
    const uint16_t mSwVersionLen = 30;
    const uint16_t mHwVersionLen = 10;
    const uint16_t mExtensionLen = 30;

    const std::vector<double> mSwVerKnown = {1.00, 2.01, 3.01};

    singleBlock_t mVersions;
    std::vector<std::string> mExtensions;

    bool mIsValid = false;
};

template <typename ClassType>
UbxMonVer<ClassType>::UbxMonVer() :
    mPayload(nullptr),
    mPayloadLen(0) {}


template <typename ClassType>
UbxMonVer<ClassType>::UbxMonVer(const char* in,
                                const size_t& inLen) :
    mPayload(in),
    mPayloadLen(inLen) {
    ALOGV("UbxMonVer: %s", __func__);

    if (UPError::Success == Parse()) {
        mIsValid = true;
    }
}

template <typename ClassType>
UPError UbxMonVer<ClassType>::Parse() {
    ALOGV("UbxMonVer: %s", __func__);

    if (nullptr == mPayload || mPayloadLen < (mSwVersionLen + mHwVersionLen)) {
        return UPError::IncompletePacket;
    }

    auto res = ParseSingleBlock();

    if (UPError::Success != res) {
        return res;
    }

    res = ParseRepeatedBlocks();

    if (UPError::Success != res) {
        return res;
    }

    res = ValidateParcel();
    return res;
}

template <typename ClassType>
UPError UbxMonVer<ClassType>::ParseSingleBlock() {
    ALOGV("UbxMonVer: %s", __func__);
    std::cmatch swStr;
    std::regex example("\\d\\.\\d\\d");

    if (std::regex_search(mPayload, swStr, example)) {
        mVersions.swVersion = atof(swStr.str().c_str());
        return UPError::Success;
    }

    return UPError::InvalidData;
}

template <typename ClassType>
UPError UbxMonVer<ClassType>::ParseRepeatedBlocks() {
    ALOGV("UbxMonVer: %s", __func__);
    uint16_t offset = ExtensionsOffset;

    while (offset + mExtensionLen <= mPayloadLen) {
        mExtensions.push_back(std::string(mPayload + offset));
        offset += mExtensionLen;
    }

    return UPError::Success;
}

template <typename ClassType>
UbxMsg UbxMonVer<ClassType>::GetMsgType() {
    return mType;
}

template <typename ClassType>
UPError UbxMonVer<ClassType>::GetData([[maybe_unused]] ClassType out) {
    return UPError::UnknownError;
}

template <typename ClassType>
bool UbxMonVer<ClassType>::IsValid() {
    return mIsValid;
}

template <typename ClassType>
UPError UbxMonVer<ClassType>::ValidateParcel() {
    auto compare = [&, this] (double a, double epsilon = 0.001) {
        return std::fabs(a - mVersions.swVersion) < epsilon;
    };

    for (auto x : mSwVerKnown) {
        ALOGV("UbxMonVer: %s, known sw", __func__);

        if (compare(x)) {
            return UPError::Success;
        }
    }

    return UPError::InvalidData;
}

template <typename T>
UbxMonVer<T>::~UbxMonVer() {
    //TODO(g.chabukiani): check if we need to clean up
}
