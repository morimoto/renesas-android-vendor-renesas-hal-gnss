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

#include "include/DeviceScanner.h"

#include <cutils/properties.h>
#include <log/log.h>
#include <poll.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/system_properties.h>
#include <unistd.h>

#include <chrono>

#include "include/DefaultReceiver.h"
#include "include/FakeReceiver.h"
#include "include/GeneralManager.h"
#include "include/GnssTransport.h"
#include "include/IUbxParser.h"
#include "include/MessageQueue.h"
#include "include/PropNames.h"
#include "include/UbloxReceiver.h"
#include "include/UbxMonVer.h"

static const int badFd = -1;
static const int bufSize = 4096;
static const int pollTimeOut = 10000; //ms
static const std::string deviceClass = "tty";

namespace android::hardware::gnss::V2_1::renesas {
static const std::set<DevId> supportedDevices = {uint16_t(VendorId::Garmin),
                                                 uint16_t(VendorId::SiRF),
                                                 uint16_t(VendorId::Ublox)};

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

VendorId toVendorId(const uint16_t value)
{
    #define CR(id) case id: return id
    switch (static_cast<VendorId>(value)) {
        CR(VendorId::SiRF);
        CR(VendorId::Garmin);
        CR(VendorId::Ublox);
        default:
            return VendorId::Unknown;
    }
}

VendorId GetVendorIdByTtyDevFile(std::string& devPath) {
    const ssize_t     maxPathSize  = PATH_MAX;
    const std::string pathTtyClass = "/sys/class/tty";
    const std::string pathDev      = "/dev/tty";
    const std::string fileUevent   = "/uevent";
    const std::string strProduct   = "PRODUCT=";
    const int         baseHex      = 16;
    const int         vidSize      = 4;
    uint16_t          vid          = 0u;
    struct stat       fileStat;
    char              charBuf[maxPathSize];

    if (devPath.find(pathDev) != 0) {
        return VendorId::Unknown;
    }
    std::string fileName = devPath.substr(devPath.rfind("/"), devPath.size());
    std::string filePath = pathTtyClass + fileName;
    // Read link
    if (lstat(filePath.c_str(), &fileStat) != 0 || !S_ISLNK(fileStat.st_mode)) {
        return VendorId::Unknown;
    }
    filePath = realpath(filePath.c_str(), &charBuf[0]);

    // Go 3 levels up directory
    for (int i = 0; i < 3; i++) {
        filePath = filePath.substr(0, filePath.rfind("/"));
    }
    // Read VID from uevent file
    fileName = filePath + fileUevent;
    std::ifstream fileIdVendor(fileName);
    while (fileIdVendor.good()) {
        fileIdVendor.getline(&charBuf[0], sizeof(charBuf));
        fileName = charBuf;
        if (fileName.find(strProduct) != 0) {
            continue;
        }
        vid = stoul(fileName.substr(strProduct.size(), vidSize), 0, baseHex);
        return toVendorId(vid);
    }

    return VendorId::Unknown;
}

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
    std::lock_guard<std::mutex> lock(mLock);

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
    std::lock_guard<std::mutex> lock(mLock);
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

    if (!IsSupportedDevice(devId) || !CheckTtyPath(path) ||
        ReceiverAlreadyAdded(path)) {
        ALOGW("Device with path %s was not inserted", path.c_str());
        return;
    }

    switch (static_cast<VendorId>(devId.vid)) {
        case VendorId::Ublox:
            CreateUbxReceiver(path, GnssReceiverType::UsbDongle,
                              Priority::UbxUsb);
            break;
        default:
            CreateDefaultReceiver(path, GnssReceiverType::UsbDongle,
                                  Priority::Default);
    }
}

void DeviceScanner::RemoveReceiver(const std::string& pathToRemove) {
    ALOGV("%s", __func__);
    std::lock_guard<std::mutex> lock(mLock);
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
    ALOGV("%s, path: %s", __func__, path.c_str());
    if (mThread.joinable()) {
        mThread.join();
    }

    mThread = std::thread([devId, state, this](const std::string path) {
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
    }, path);

    if (!mGManager->IsRun() && mThread.joinable()) {
        mThread.join();
    }
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

DSError DeviceScanner::CreateUbxReceiver(const std::string& path,
        const GnssReceiverType& type, const Priority& priority) {
    ALOGI("Creation of U-Blox receiver with path: %s", path.c_str());
    std::lock_guard<std::mutex> lock(mLock);
    std::shared_ptr<UbloxReceiver> ubReceiver = std::make_shared<UbloxReceiver>
                                                (path, type);
    receiver_t receiver = {ubReceiver, priority};
    size_t emptyLen = mDefaultPropertyValue.length();

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
    ALOGI("Creation of general receiver with path: %s", path.c_str());
    std::lock_guard<std::mutex> lock(mLock);
    std::shared_ptr<DefaultReceiver> dfReceiver = std::make_shared<DefaultReceiver>
                                                  (path, type);
    receiver_t receiver = {dfReceiver, priority};

    dfReceiver->SetBaudRate(static_cast<uint32_t>(mSettings.requestedBaudrate));
    mReceivers.push(receiver);
    return DSError::Success;
}

DSError DeviceScanner::CreateFakeReceiver() {
    ALOGI("Creation of fake receiver");
    std::lock_guard<std::mutex> lock(mLock);

    receiver_t receiver =
            {std::make_shared<FakeReceiver>(mFakeRoutePath), Priority::Fake};
    mReceivers.push(receiver);
    return DSError::FakeReceiver;
}

std::shared_ptr<IGnssReceiver> DeviceScanner::GetReceiver() {
    ALOGV("%s", __func__);
    std::lock_guard<std::mutex> lock(mLock);

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
    std::string ttyPath = mSettings.ttyPath;

    if (mFake == ttyPath) {
        return CreateFakeReceiver();
    }

    if (ttyPath.size() == 0 || !CheckTtyPath(ttyPath)) {
        return DSError::NoPredefinedReceiver;
    }

    if (GetVendorIdByTtyDevFile(ttyPath) == VendorId::Ublox) {
        return CreateUbxReceiver(mSettings.ttyPath, GnssReceiverType::UsbDongle,
                                 Priority::Requested);
    } else {
        return CreateDefaultReceiver(mSettings.ttyPath,
                                     GnssReceiverType::UsbDongle,
                                     Priority::Requested);
    }
}

DSError DeviceScanner::CheckPredefinedSettings() {
    ALOGV("%s", __func__);
    char ttyPath[PROPERTY_VALUE_MAX] = {};
    char secmajor[PROPERTY_VALUE_MAX] = {};
    char sbas[PROPERTY_VALUE_MAX] = {};

    property_get(propRequestedReceiver.c_str(), ttyPath,
                 mDefaultPropertyValue.c_str());
    property_get(propSecmajor.c_str(), secmajor,
                 mDefaultPropertyValue.c_str());
    property_get(propSbas.c_str(), sbas, mDefaultPropertyValue.c_str());
    mSettings.ttyPath = std::string(ttyPath);
    mSettings.secmajor = std::string(secmajor);
    mSettings.sbas = std::string(sbas);
    mSettings.requestedBaudrate = property_get_int32(propBaudRate.c_str(),
                                                     mTtyDefaultRate);

    return DSError::Success;
}

bool DeviceScanner::HandleNotifyMessage(const int fd, const std::string& fileName) {
    ALOGV("%s", __func__);
    const inotify_event* event = nullptr;
    char buf[bufSize] = {0};

    for (int ret = read(fd, buf, bufSize);
         ret > 0;
         ret = read(fd, buf, bufSize)) {
        for (char* crntEvent = buf; crntEvent < buf + ret;
             crntEvent += sizeof(inotify_event) + event->len) {
            event = reinterpret_cast<const inotify_event*>(buf);
            ALOGV("event->len: %d, event->name: %s", event->len, event->name);
            if (!fileName.compare(event->name)) {
                ALOGV("Found : %s", event->name);
                return true;
            }
        }
    }

    return false;
}

bool DeviceScanner::IsSupportedDevice(DevId& devId) {
    for (auto supDev = supportedDevices.begin();
         supDev != supportedDevices.end(); supDev++) {
        if (supDev->vid == devId.vid) {
            return true;
        }
    }
    return false;
}

bool DeviceScanner::WaitEventFromSystem(const std::string& path) {
    ALOGV("%s", __func__);
    bool res = false;
    int watchDesriptor = 0, poll_num = 0;
    nfds_t numFileDscrptrs = 1;
    std::string watchDir, fileName;
    netlink_desc fileDescriptor(inotify_init1(IN_NONBLOCK));
    pollfd fds = {
        .fd = fileDescriptor,
        .events = POLLIN,
    };

    try {
        watchDir = path.substr(0, path.rfind('/'));
        fileName = path.substr(path.rfind('/') + 1);
        ALOGV("watchdir: %s, filename: %s", watchDir.c_str(), fileName.c_str());
    } catch (const std::exception& e) {
        ALOGW("Substr: %s", e.what());
        return false;
    }

    if (badFd == fileDescriptor) {
        ALOGW("Error: inotify_init1");
        return false;
    }

    watchDesriptor = inotify_add_watch(fileDescriptor, watchDir.c_str(), IN_CREATE);
    if (badFd == watchDesriptor) {
        ALOGW("Error: Cannot watch: %s, %s", watchDir.c_str(), strerror(errno));
        return false;
    }

    ALOGV("Listening for events...");

    while (true) {
        poll_num = poll(&fds, numFileDscrptrs, pollTimeOut);

        if (poll_num == -1) {
            ALOGW("Poll failed: %s", std::strerror(errno));
            break;
        }

        if (poll_num == 0) {
            ALOGW("Event not received after %d ms waiting", pollTimeOut);
            break;
        }

        if (fds.revents & POLLIN) {
            ALOGV("Received event from system");
            res = HandleNotifyMessage(fileDescriptor, fileName);
            if (res == true) {
                break;
            }
        }
    }

    return (res);
}

bool DeviceScanner::CheckTtyPath(const std::string& path) {
    ALOGV("%s", __func__);
    if (!access(path.c_str(), F_OK)){
        ALOGV("File %s - exist, return true", path.c_str());
        return (true);
    }

    ALOGV("File %s - NO exist, wait notify from system...", path.c_str());
    return WaitEventFromSystem(path);
}

} //namespace android::hardware::gnss::V2_1::renesas