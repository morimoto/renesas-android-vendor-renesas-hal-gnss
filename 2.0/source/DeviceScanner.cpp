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
#define LOG_TAG "GnssRenesasHalDeviceScanner"
#define LOG_NDEBUG 1

#include <chrono>
#include <unistd.h>
#include <log/log.h>

#include "include/DeviceScanner.h"

#include <cutils/properties.h>
#include <sys/system_properties.h>

#include "include/DefaultReceiver.h"
#include "include/FakeReceiver.h"
#include "include/GeneralManager.h"
#include "include/GnssTransport.h"
#include "include/IUbxParser.h"
#include "include/MessageQueue.h"
#include "include/UbloxReceiver.h"
#include "include/UbxMonVer.h"

static const uint16_t ubxVid = 0x1546;
static const int maxCheckRetries = 5;
static const int tryInterval = 50;
static const std::string deviceClass = "tty";
static const std::set<DevId> supportedDevices = {ubxVid};

namespace android::hardware::gnss::V2_0::renesas {
// TODO (d.gamazin) Move to separate header
const std::string DeviceScanner::mPropRequestedReceiver = "ro.boot.gps.mode";
const std::string DeviceScanner::mPropBaudRate = "ro.boot.gps.tty_baudrate";
const std::string DeviceScanner::mPropSecmajor = "ro.boot.gps.secmajor";
const std::string DeviceScanner::mPropSbas = "ro.boot.gps.sbas";

// SC3 hardcoded in device tree overlay
const std::string DeviceScanner::mSkKfUbloxTtyPath = "/dev/ttySC3";
// Ublox represantation, the only available in ueventd.rc
const std::string DeviceScanner::mSalvatorUbloxTtyPath = "/dev/ttyACM0";
// SiRF represantation, the only available in ueventd.rc
const std::string DeviceScanner::mSalvatorOtherTtyPath = "/dev/ttyUSB0";
// To enable fake location provider
const std::string DeviceScanner::mFake = "fake";

const std::string DeviceScanner::mFakeRoutePath = "/vendor/etc/fake_route.txt";

const std::string DeviceScanner::mDefaultPropertyValue = "";

const int32_t DeviceScanner::mTtyDefaultRate = 9600;

DSError DeviceScanner::DetectDevice() {
    ALOGV("%s", __func__);
    char propBuf[PROPERTY_VALUE_MAX] = {};
    std::string propHardware("ro.hardware");
    std::string kingfisher("kingfisher");
    int result = __system_property_get(propHardware.c_str(), propBuf);

    if (result > 0) {
        if (0 == kingfisher.compare(propBuf)) {
            mBoard = TargetDevice::Kingfisher;
        } else {
            mBoard = TargetDevice::Salvator;
        }

        mTargetDevice = std::string(propBuf);
        return DSError::Success;
    }

    return DSError::InternalError;
}

void DeviceScanner::PrintReceivers() {
    receiversQueue_t Receivers = mReceivers;
    ALOGV("mReceivers.size(): %lu", mReceivers.size());

    while (!Receivers.empty()) {
        std::string path;
        path = Receivers.top().receiver->GetTransport()->GetPath();
        ALOGV("receiver path : %s", path.c_str());
        Receivers.pop();
    }
}

bool DeviceScanner::ReceiverAlreadyAdded(const std::string& newPath) {
    ALOGV("%s", __func__);
    receiversQueue_t Receivers = mReceivers;

    while (!Receivers.empty()) {
        std::string path;
        path = Receivers.top().receiver->GetTransport()->GetPath();

        if (path == newPath) {
            return true;
        }

        Receivers.pop();
    }

    return false;
}

void DeviceScanner::InsertNewReceiver(const std::string& path, DevId devId) {
    ALOGV("%s", __func__);
    switch (devId.vid) {
        case (static_cast<uint16_t>(VendorId::Ublox)): {
            if (CheckTtyPath(path) && !ReceiverAlreadyAdded(path)) {
                CreateUbxReceiver(path, GnssReceiverType::UsbDongle,
                                  Priority::UbxUsb);
            }
            break;
        }
        default: {
            break;
        }
    }
}

void DeviceScanner::RemoveReceiver(const std::string& pathToRemove) {
    ALOGV("%s", __func__);
    receiversQueue_t newReceivers;

    while (!mReceivers.empty()) {
        std::string path;
        path = mReceivers.top().receiver->GetTransport()->GetPath();

        if (path != pathToRemove) {
            newReceivers.push(mReceivers.top());
        }

        mReceivers.pop();
    }

    mReceivers = newReceivers;
}

void DeviceScanner::UpdateReceivers(const std::string& path, DevId devId,
                                    ProbingStage state) {
    ALOGV("%s", __func__);
    mGManager->StopToChange();
    switch (state) {
        case ProbingStage::Added: {
            InsertNewReceiver(path, devId);
            break;
        }

        case ProbingStage::Removed: {
            RemoveReceiver(path);
            break;
        }

        default: {
            break;
        }
    }
    mGManager->StartAfterChange();
}

bool DeviceScanner::IsKingfisher() {
    return (TargetDevice::Kingfisher == mBoard);
}

DeviceScanner::DeviceScanner(GeneralManager* gManager)
    : mGManager(gManager),
      mUscanner(supportedDevices, deviceClass,
                [this](const std::string& path, DevId devId, ProbingStage st) {
    UpdateReceivers(path, devId, st);}) {
    DetectDevice();
}

DeviceScanner::~DeviceScanner() {
    mUscanner.Stop();
}

DSError DeviceScanner::Scan() {
    ALOGV("%s", __func__);
    CheckPredefinedSettings();
    ProcessPredefinedSettings();
    if (IsKingfisher() && CheckTtyPath(mSkKfUbloxTtyPath)) {
        CreateUbxReceiver(mSkKfUbloxTtyPath,
                          GnssReceiverType::OnboardChip, Priority::UbxNative);
    }

    auto res = mUscanner.Start();
    if (ProbingStage::Error == res) {
        return DSError::InternalError;
    }

    if (0 == mReceivers.size()) {
        return DSError::NoReceiver;
    }

    return DSError::Success;
}

//TODO(g.chabukiani): set some nessessary params, such as baud rate etc.
DSError DeviceScanner::CreateUbxReceiver(const std::string& path,
        const GnssReceiverType& type, const Priority& priority) {
    ALOGV("%s", __func__);
    std::shared_ptr<UbloxReceiver> ubReceiver = std::make_shared<UbloxReceiver>
                                                (path, type);
    receiver_t receiver = {ubReceiver, priority};
    size_t emptyLen = mDefaultPropertyValue.length();
    ubReceiver->SetBaudRate(static_cast<uint32_t>
                            (mSettings.requestedBaudrate));

    if (mSettings.secmajor.length() > emptyLen) {
        receiver.receiver->SetSecondMajorConstellation(mSettings.secmajor);
    }

    if (mSettings.sbas.length() > emptyLen) {
        receiver.receiver->SetSbasStatus(mSettings.sbas);
    }

    mReceivers.push(receiver);
    return DSError::Success;
}

DSError DeviceScanner::CreateDefaultReceiver(const std::string& path,
        const GnssReceiverType& type, const Priority& priority) {
    ALOGV("%s", __func__);
    std::shared_ptr<DefaultReceiver> dfReceiver = std::make_shared<DefaultReceiver>
                                                  (path, type);
    receiver_t receiver = {dfReceiver, priority};
    dfReceiver->SetBaudRate(static_cast<uint32_t>
                            (mSettings.requestedBaudrate));
    mReceivers.push(receiver);
    return DSError::Success;
}

DSError DeviceScanner::CreateFakeReceiver() {
    ALOGV("%s", __func__);
    receiver_t receiver =
            {std::make_shared<FakeReceiver>(mFakeRoutePath), Priority::Fake};
    mReceivers.push(receiver);
    return DSError::FakeReceiver;
}

std::shared_ptr<IGnssReceiver> DeviceScanner::GetReceiver() {
    ALOGV("%s", __func__);

    if (0 < mReceivers.size()) {
        std::string path;
        receiver_t receiver = mReceivers.top();
        path = receiver.receiver->GetTransport()->GetPath();
        ALOGV("%s, receiver path: %s", __func__, path.c_str());
        return receiver.receiver;
    }

    return nullptr;
}

DSError DeviceScanner::ProcessPredefinedSettings() {
    ALOGV("%s", __func__);

    if (mFake == mSettings.ttyPath && CheckTtyPath(mFakeRoutePath)) {
        return CreateFakeReceiver();
    }

    if (mSettings.ttyPath.length() > mDefaultPropertyValue.length()
        && CheckTtyPath(mSettings.ttyPath)) {
        if (mSkKfUbloxTtyPath == mSettings.ttyPath && IsKingfisher()) {
            return CreateUbxReceiver(mSkKfUbloxTtyPath,
                    GnssReceiverType::OnboardChip, Priority::Requested);
        } else if (mSalvatorUbloxTtyPath == mSettings.ttyPath) {
            return CreateUbxReceiver(mSalvatorUbloxTtyPath,
                    GnssReceiverType::UsbDongle, Priority::Requested);
        } else if (mSalvatorOtherTtyPath == mSettings.ttyPath) {
            return CreateDefaultReceiver(mSalvatorOtherTtyPath,
                    GnssReceiverType::UsbDongle, Priority::Requested);
        } else  {
            return DSError::UnsupportedReceiver;
        }
    }

    return DSError::NoPredefinedReceiver;
}

DSError DeviceScanner::CheckPredefinedSettings() {
    ALOGV("%s", __func__);
    char propTty[PROPERTY_VALUE_MAX] = {};
    char propSecmajor[PROPERTY_VALUE_MAX] = {};
    char propSbas[PROPERTY_VALUE_MAX] = {};
    property_get(mPropRequestedReceiver.c_str(), propTty,
                 mDefaultPropertyValue.c_str());
    property_get(mPropSecmajor.c_str(), propSecmajor,
                 mDefaultPropertyValue.c_str());
    property_get(mPropSbas.c_str(), propSbas, mDefaultPropertyValue.c_str());
    mSettings.ttyPath = std::string(propTty);
    mSettings.secmajor = std::string(propSecmajor);
    mSettings.sbas = std::string(propSbas);
    mSettings.requestedBaudrate = property_get_int32(mPropBaudRate.c_str(),
                                                     mTtyDefaultRate);

    return DSError::Success;
}

bool DeviceScanner::CheckTtyPath(const std::string& path) {
    ALOGV("%s", __func__);
    int res;
    for (int retry = 0; retry < maxCheckRetries; ++retry) {
        res = access(path.c_str(), F_OK);
        if (res != 0) {
            //waiting for the receiver to be ready after plug.
            std::this_thread::sleep_for(std::chrono::milliseconds(tryInterval));
        } else {
            break;
        }
    }

    return (res == 0);
}

} //namespace android::hardware::gnss::V2_0::renesas
