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
#define LOG_TAG "GnssRenesasHalDefaultReceiver"
#define LOG_NDEBUG 1

#include "include/DefaultReceiver.h"

#include <type_traits>
#include <log/log.h>

#include "include/GnssTransportTTY.h"

namespace android::hardware::gnss::V2_0::renesas {

DefaultReceiver::DefaultReceiver(const std::string& path,
                                 const GnssReceiverType& type) :
    GnssReceiverTTY(new GnssTransportTTY(path)),
    mReceiverType(type), mTtyPath(path)  {
    ALOGV("%s", __func__);
}

DefaultReceiver::DefaultReceiver(uint16_t vendorId,
                                 uint16_t productId,
                                 const std::string& path,
                                 const GnssReceiverType& type) :
    GnssReceiverTTY(new GnssTransportTTY(path)),
    mReceiverType(type),
    mVendorId(vendorId),
    mProductId(productId),
    mTtyPath(path) {
    ALOGV("%s", __func__);
}

DefaultReceiver::~DefaultReceiver() {}

GnssReceiverType DefaultReceiver::GetReceiverType() {
    ALOGV("%s", __func__);
    return mReceiverType;
}

GnssVendor DefaultReceiver::GetVendorId() {
    ALOGV("%s", __func__);
    return mVendor;
}

GnssProduct DefaultReceiver::GetProductId() {
    ALOGV("%s", __func__);
    (void) mProductId;
    return mProduct;
}

uint32_t DefaultReceiver::GetYearOfHw() {
    ALOGV("%s", __func__);
    return mYearOfHw;
}

RError DefaultReceiver::GetFirmwareVersionStr(std::string& out) {
    ALOGV("%s", __func__);

    if (mFirmwareVersion.empty()) {
        return RError::Unknown;
    }

    out = mFirmwareVersion;
    return RError::Success;
}

float DefaultReceiver::GetFirmwareVersion() {
    ALOGV("%s", __func__);
    return mFirmware;
}

RError DefaultReceiver::GetVendorName(std::string& out) {
    ALOGV("%s", __func__);

    if (mVendorName.empty()) {
        return RError::Unknown;
    }

    out = mVendorName;
    return RError::Success;
}

RError DefaultReceiver::GetProductName(std::string& out) {
    ALOGV("%s", __func__);

    if (mProductName.empty()) {
        return RError::Unknown;
    }

    out = mProductName;
    return RError::Success;
}

void DefaultReceiver::GetSupportedProtocols(
    std::vector<SupportedProtocol>& out) {
    ALOGV("%s", __func__);
    out.push_back(mProtocol);
}

uint32_t DefaultReceiver::GetBaudRate() {
    ALOGV("%s", __func__);
    return mLineBaudRate;
}

RError DefaultReceiver::SetVendor() {
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

RError DefaultReceiver::SetBaudRate(const uint32_t& baudrate) {
    ALOGV("%s", __func__);

    if (baudrate < minBaudRate && baudrate > maxBaudRate) {
        return RError::NotSupported;
    }

    mLineBaudRate = baudrate;
    return RError::Success;
}

RError DefaultReceiver::SetFwVersion(const double& version) {
    ALOGV("%s", __func__);
    mFirmware = static_cast<float>(version);
    return RError::Success;
}


RError DefaultReceiver::SetSecondMajorConstellation([[maybe_unused]] const
        std::string& constellation) {
    ALOGV("%s", __func__);
    return RError::Success;
}

RError DefaultReceiver::SetSbasStatus([[maybe_unused]] const std::string&
                                      status) {
    ALOGV("%s", __func__);
    return RError::Success;
}


SWVersion DefaultReceiver::GetSwVersion() {
    ALOGV("%s", __func__);
    return mSwVersion;
}

} // namespace android::hardware::gnss::V2_0::renesas
