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
#include <cstdint>
#include <string>
#include <vector>
#include <unistd.h>

#include "include/GnssTransport.h"

namespace android::hardware::gnss::V2_1::renesas {

enum class RError : uint8_t {
    Success,
    InternalError,
    NotSupported,
    Unknown
};

enum class GnssVendor : uint8_t {
    Ublox,
    SiRF,
    MTK,
    Broadcom,
    Fake,
    Unknown,
};

enum class GnssProduct : uint8_t {
    M8Q001,
    M8Q010,
    M8030KT,
    M7000,
    Fake,
    Undefined,
    SiRFIV
};

enum class GnssReceiverType : uint8_t {
    UsbDongle,
    FakeReceiver,
    OnboardChip,
};

enum class SupportedProtocol : uint8_t {
    NMEA0183,
    UbxBinaryProtocol,
    SirfBinaryProtocol,
    FakeReceiverProtocol,
    UnknownProtocol,
};

enum class VendorId : uint16_t {
    SiRF   = 0x067b,
    Garmin = 0x091E,
    Ublox  = 0x1546,
    Unknown,
};

VendorId toVendorId(const uint16_t value);

enum SWVersion : uint16_t {
    SPG_100 = 0,
    SPG_201 = 1,
    SPG_301 = 2,
    HPG_140,
    TIM_110,
    Unknown,
};

const uint32_t minBaudRate = 4800;
const uint32_t maxBaudRate = 460800;

class IGnssReceiver {
public:
    /*!
     * \brief IGnssReceiver
     */
    IGnssReceiver() = default;

    /*!
     * \brief ~IGnssReceiver
     */
    virtual ~IGnssReceiver() = default;

    /*!
     * \brief GetReceiverType
     * \return
     */
    virtual GnssReceiverType GetReceiverType() = 0;

    /*!
     * \brief GetVendorId
     * \return
     */
    virtual GnssVendor GetVendorId() = 0;

    /*!
     * \brief GetProductId
     * \return
     */
    virtual GnssProduct GetProductId() = 0;

    /*!
     * \brief GetYearOfHw
     * \return
     */
    virtual uint32_t GetYearOfHw() = 0;

    /*!
     * \brief GetFirmwareVersionStr
     * \return
     */
    virtual RError GetFirmwareVersionStr(std::string& out) = 0;

    /*!
     * \brief GetFirmwareVersion
     * \return
     */
    virtual float GetFirmwareVersion() = 0;

    /*!
     * \brief GetVendorName
     * \return
     */
    virtual RError GetVendorName(std::string& out) = 0;

    /*!
     * \brief GetProductName
     * \return
     */
    virtual RError GetProductName(std::string& out) = 0;

    /*!
     * \brief GetSupportedProtocols
     * \param out
     */
    virtual void
    GetSupportedProtocols(std::vector<SupportedProtocol>& out) = 0;

    /*!
     * \brief SetFwVersion
     * \param version
     * \return
     */
    virtual RError SetFwVersion(const double& version) = 0;

    /*!
     * \brief SetSecondMajorConstellation
     * \param constellation
     * \return
     */
    virtual RError SetSecondMajorConstellation(const std::string&
            constellation) = 0;

    /*!
     * \brief SetSbasStatus
     * \param status
     * \return
     */
    virtual RError SetSbasStatus(const std::string& status) = 0;

    /*!
     * \brief GetSwVersion
     * \return
     */
    virtual SWVersion GetSwVersion() = 0;

    virtual std::shared_ptr<Transport> GetTransport() = 0;
};

} // namespace android::hardware::gnss::V2_0::renesas
