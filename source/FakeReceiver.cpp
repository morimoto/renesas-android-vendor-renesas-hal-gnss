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

#include <FakeReceiver.h>

#include <GnssTransportFake.h>

namespace android::hardware::gnss::V2_1::renesas{

FakeReceiver::FakeReceiver(const std::string& path) :
    GnssReceiver(new GnssTransportFake(path)) {
}

GnssReceiverType FakeReceiver::GetReceiverType() {
    return mReceiverType;
}

GnssVendor FakeReceiver::GetVendorId() {
    return mVendor;
}

GnssProduct FakeReceiver::GetProductId() {
    return mProduct;
}

uint32_t FakeReceiver::GetYearOfHw() {
    return mYearOfHw;
}

RError FakeReceiver::GetFirmwareVersionStr(std::string& out) {
    if (mFirmwareVersion.empty()) {
        return RError::Unknown;
    }

    out = mFirmwareVersion;
    return RError::Success;
}

float FakeReceiver::GetFirmwareVersion() {
    return mFirmware;
}

RError FakeReceiver::GetVendorName(std::string& out) {
    if (mVendorName.empty()) {
        return RError::Unknown;
    }

    out = mVendorName;
    return RError::Success;
}

RError FakeReceiver::GetProductName(std::string& out) {
    if (mProductName.empty()) {
        return RError::Unknown;
    }

    out = mProductName;
    return RError::Success;
}

void FakeReceiver::GetSupportedProtocols(std::vector<SupportedProtocol>& out) {
    out.push_back(mProtocol);
}

RError FakeReceiver::SetFwVersion(const double& version) {
    mFirmware = static_cast<float>(version);
    return RError::Success;
}

RError FakeReceiver::SetSecondMajorConstellation([[maybe_unused]] const
        std::string& constellation) {
    return RError::Success;
}

RError FakeReceiver::SetSbasStatus([[maybe_unused]] const
        std::string& status) {
    return RError::Success;
}

SWVersion FakeReceiver::GetSwVersion() {
    return mSwVersion;
}

} // namespace android::hardware::gnss::V2_1::renesas
