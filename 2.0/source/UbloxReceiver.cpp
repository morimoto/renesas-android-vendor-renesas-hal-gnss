/*
 * Copyright (C) 2020 GlobalLogic
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
{}
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

#define LOG_TAG "GnssRenesasHalUbloxReceiver"
#define LOG_NDEBUG 1

#include <log/log.h>
#include <type_traits>
#include "include/UbloxReceiver.h"

void UbloxReceiver::SetSupportedProtocols() {
    ALOGV("%s", __func__);
    mProtocolList.push_back(SupportedProtocol::UbxBinaryProtocol);
    mProtocolList.push_back(SupportedProtocol::NMEA0183);
}

UbloxReceiver::UbloxReceiver(uint16_t vendorId, uint16_t productId,
                             const GnssReceiverType& type) :
    mReceiverType(type),
    mVendorId(vendorId),
    mProductId(productId) {
    ALOGV("%s", __func__);
    SetSupportedProtocols();
}

UbloxReceiver::UbloxReceiver(uint16_t vendorId, uint16_t productId,
                             std::string& path, const GnssReceiverType& type) :
    mReceiverType(type),
    mVendorId(vendorId),
    mProductId(productId),
    mTtyPath(path) {
    ALOGV("%s", __func__);
    SetSupportedProtocols();
}

UbloxReceiver::~UbloxReceiver()
{}

GnssReceiverType UbloxReceiver::GetReceiverType() {
    ALOGV("%s", __func__);
    return mReceiverType;
}

GnssProduct UbloxReceiver::GetProductId() {
    ALOGV("%s", __func__);
    (void) mProductId;
    return mProduct;
}

uint32_t UbloxReceiver::GetYearOfHw() {
    ALOGV("%s", __func__);
    return mYearOfHw;
}

RError UbloxReceiver::GetFirmwareVersionStr(std::string& out) {
    ALOGV("%s", __func__);

    if (mFirmwareVersion.empty()) {
        return RError::Unknown;
    }

    out = mFirmwareVersion;
    return RError::Success;
}

float UbloxReceiver::GetFirmwareVersion() {
    ALOGV("%s", __func__);
    return mFirmware;
}

RError UbloxReceiver::GetVendorName(std::string& out) {
    ALOGV("%s", __func__);

    if (mVendorName.empty()) {
        return RError::Unknown;
    }

    out = mVendorName;
    return RError::Success;
}

RError UbloxReceiver::GetProductName(std::string& out) {
    ALOGV("%s", __func__);

    if (mProductName.empty()) {
        return RError::Unknown;
    }

    out = mProductName;
    return RError::Success;
}

RError UbloxReceiver::GetPath(std::string& out) {
    ALOGV("%s", __func__);

    if (mTtyPath.empty()) {
        return RError::Unknown;
    }

    out = mTtyPath;
    return RError::Success;
}

void UbloxReceiver::GetSupportedProtocols(std::vector<SupportedProtocol>& out) {
    ALOGV("%s", __func__);

    for (auto protocol : mProtocolList) {
        out.push_back(protocol);
    }
}

template <typename T>
RError UbloxReceiver::GetUbxProtocolVersion(T& out) {
    ALOGV("%s", __func__);

    if constexpr(std::is_same_v<uint32_t, T> && mUbloxPrVersionMax != 0) {
        out = mUbloxPrVersionMax;
    } else if constexpr(std::is_same_v<std::string, T>
                        && !mUbloxProtocolMaxVersion.empty()) {
        out = mUbloxProtocolMaxVersion;
    } else {
        return RError::Unknown;
    }

    return RError::Success;
}

GnssVendor UbloxReceiver::GetVendorId() {
    ALOGV("%s", __func__);
    return mVendor;
}

uint32_t UbloxReceiver::GetBaudRate() {
    ALOGV("%s", __func__);
    return mLineBaudRate;
}

RError UbloxReceiver::SetVendor() {
    ALOGV("%s", __func__);

    switch (mVendorId) {
    case static_cast<decltype (mVendorId)>(VendorId::Ublox):
        mVendor = GnssVendor::Ublox;
        break;

    case static_cast<decltype (mVendorId)>(VendorId::SiRF):
        mVendor = GnssVendor::SiRF;
        break;

    case static_cast<decltype (mVendorId)>(VendorId::Unknown):
    default:
        mVendor = GnssVendor::Unknown;
        return RError::Unknown;
    }

    return RError::Success;
}

UbloxReceiver::UbloxReceiver(const std::string& path,
                             const GnssReceiverType& type) :
    mReceiverType(type),
    mTtyPath(path) {
    ALOGV("%s", __func__);
    SetSupportedProtocols();
}

RError UbloxReceiver::SetBaudRate(const uint32_t& baudrate) {
    ALOGV("%s", __func__);

    if (baudrate < minBaudRate && baudrate > maxBaudRate) {
        return RError::NotSupported;
    }

    mLineBaudRate = baudrate;
    return RError::Success;
}

RError UbloxReceiver::SetFwVersion(const double& version) {
    ALOGV("%s", __func__);
    auto compare = [&] (double a, double epsilon = 0.001) {
        return (std::fabs(a - version) < epsilon);
    };

    if (compare(SPG_100)) {
        mSwVersion = SWVersion::SPG_100;
    } else if (compare(SPG_201)) {
        mSwVersion = SWVersion::SPG_201;
    } else if (compare(SPG_301)) {
        mSwVersion = SWVersion::SPG_301;
    } else {
        return RError::NotSupported;
    }

    mFirmware = static_cast<float>(version);
    return RError::Success;
}

RError UbloxReceiver::SetSecondMajorConstellation(const std::string&
        constellation) {
    ALOGV("%s", __func__);
    mSecondMajorConstellation = constellation;
    return RError::Success;
}

RError UbloxReceiver::SetSbasStatus(const std::string& status) {
    ALOGV("%s", __func__);

    if (status.empty()) {
        return RError::InternalError;
    }

    if (status == "disabled") {
        mSbasStatus = status;
        mSbasEnabled = false;
    } else if (status == "enabled") {
        mSbasStatus = status;
        mSbasEnabled = true;
    } else {
        return RError::InternalError;
    }

    return RError::Success;
}

SWVersion UbloxReceiver::GetSwVersion() {
    ALOGV("%s", __func__);
    return mSwVersion;
}
