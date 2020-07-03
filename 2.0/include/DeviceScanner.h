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
#pragma once

#include <queue>
#include <vector>
#include <memory>
#include <mutex>

#include "include/IGnssReceiver.h"
#include <usb-scanner/UsbScanner.h>

namespace android::hardware::gnss::V2_0::renesas {
class GeneralManager;

enum class Priority : uint8_t {
    Fake = 1,
    UbxUsb,
    UbxNative,
    Requested,
    Default,
};

enum class DSError : uint8_t {
    Success,
    NoReceiver,
    UnsupportedReceiver,
    FakeReceiver,
    NoPredefinedReceiver,
    InternalError,
};

enum class TargetDevice : uint8_t {
    Salvator,
    Kingfisher,
    Unknown,
};

class DeviceScanner {
    typedef struct Receiver {
        std::shared_ptr<IGnssReceiver> receiver;
        Priority priority;
    } receiver_t;

    //TODO(g.chabukiani): check if this comparison is expected,
    //(maybe should be [left < right])
    struct Compare {
        bool operator()(const receiver_t& left, const receiver_t& right) const {
            return left.priority > right.priority;
        }
    };

    using receiversQueue_t = std::priority_queue<receiver_t, std::vector<receiver_t>, Compare>;
public:
    DeviceScanner(GeneralManager* gManager);
    virtual ~DeviceScanner();

    /*!
     * \brief GetReceiver
     * \return
     */
    std::shared_ptr<IGnssReceiver> GetReceiver();

    /*!
     * \brief Scan
     * \return
     */
    DSError Scan();

protected:
    /*!
     * \brief CreateFakeReceiver
     * \param type
     * \return
     */
    DSError CreateFakeReceiver();

    /*!
     * \brief CreateUbxReceiver
     * \param path
     * \return
     */
    DSError CreateUbxReceiver(const std::string& path,
                              const GnssReceiverType& type, const Priority& priority);

    /*!
     * \brief CreateDefaultReceiver
     * \param path
     * \return
     */
    DSError CreateDefaultReceiver(const std::string& path,
                                  const GnssReceiverType& type, const Priority& priority);

    /*!
     * \brief CheckPredefinedSettings
     * \return
     */
    DSError CheckPredefinedSettings();

    /*!
     * \brief ProcessPredefinedSettings
     * \return
     */
    DSError ProcessPredefinedSettings();

    /*!
     * \brief CheckTtyPath
     * \param path
     * \return
     */
    bool CheckTtyPath(const std::string& path);

    /*!
     * \brief DetectDevice - get target board name
     * \return
     */
    DSError DetectDevice();

    /*!
     * \brief IsKingfisher
     * \return
     */
    bool IsKingfisher();

    /*!
     * \brief ReceiverAlreadyAdded
     * \return
     */
    bool ReceiverAlreadyAdded(const std::string& newPath);

    /*!
     * \brief InsertNewReceiver
     * \return
     */
    void InsertNewReceiver(const std::string& path, DevId devId);

    /*!
     * \brief RemoveReceiver
     * \return
     */
    void RemoveReceiver(const std::string& pathToRemove);

    /*!
     * \brief UpdateReceivers - method which call from UsbScanner
     * to update receivers queue
     * \return
     */
    void UpdateReceivers(const std::string&, DevId, ProbingStage);

    /*!
     * \brief PrintReceivers
     * \return
     */
    void PrintReceivers();

    /*!
     * \brief WaitEventFromSystem - a method in which we expect
     * a message from the system that the device is ready
     * \param path - path to the expected file
     * \return
     */
    bool WaitEventFromSystem(const std::string& path);

    /*!
     * \brief HandleNotifyMessage - method for getting a message from
     * the notify event
     * \param fileName - the name of expected file
     * \param fd - file descriptor
     * \return
     */
    bool HandleNotifyMessage(const int fd, const std::string& fileName);

private:
    typedef struct PredefinedSettings {
        std::string ttyPath;
        std::string secmajor;
        std::string sbas;
        uint32_t requestedBaudrate;
    } predefinedSettings_t;

    static const std::string mPropRequestedReceiver;
    static const std::string mDefaultPropertyValue;
    static const std::string mPropBaudRate;
    static const std::string mPropSecmajor;
    static const std::string mPropSbas;
    static const int32_t mTtyDefaultRate;

    static const std::string mSalvatorUbloxTtyPath;
    static const std::string mSalvatorOtherTtyPath;
    static const std::string mSkKfUbloxTtyPath;
    static const std::string mFakeRoutePath;
    static const std::string mFake;

    std::mutex mLock;
    std::thread mThread;
    std::string mTargetDevice;
    TargetDevice mBoard = TargetDevice::Unknown;
    GeneralManager* mGManager;
    predefinedSettings_t mSettings = {};
    receiversQueue_t mReceivers;
    UsbScanner mUscanner;
};
}
