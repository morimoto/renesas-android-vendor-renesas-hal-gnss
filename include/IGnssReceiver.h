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

#ifndef IGNSSRECEIVER_H
#define IGNSSRECEIVER_H

#include <GnssTransport.h>

namespace android::hardware::gnss::V2_1::renesas {

/**
 * @brief return error type
 */
enum class RError : uint8_t {
    /**
     * @brief Success
     */
    Success,

    /**
     * @brief Internal Error
     */
    InternalError,

    /**
     * @brief Not Supported
     */
    NotSupported,

    /**
     * @brief Unknown error
     */
    Unknown
};

/**
 * @brief Gnss Vendor
 */
enum class GnssVendor : uint8_t {
    /**
     * @brief Ublox
     */
    Ublox,

    /**
     * @brief SiRF
     */
    SiRF,

    /**
     * @brief MTK
     */
    MTK,

    /**
     * @brief Broadcom
     */
    Broadcom,

    /**
     * @brief Fake
     */
    Fake,

    /**
     * @brief Unknown
     */
    Unknown,
};

/**
 * @brief Gnss Product
 */
enum class GnssProduct : uint8_t {
    /**
     * @brief M8Q001
     */
    M8Q001,

    /**
     * @brief M8Q010
     */
    M8Q010,

    /**
     * @brief M8030KT
     */
    M8030KT,

    /**
     * @brief M7000
     */
    M7000,

    /**
     * @brief Fake
     */
    Fake,

    /**
     * @brief Undefined
     */
    Undefined,

    /**
     * @brief SiRFIV
     */
    SiRFIV
};

/**
 * @brief Gnss Receiver Type
 */
enum class GnssReceiverType : uint8_t {
    /**
     * @brief UsbDongle
     */
    UsbDongle,

    /**
     * @brief FakeReceiver
     */
    FakeReceiver,

    /**
     * @brief OnboardChip
     */
    OnboardChip,
};

/**
 * @brief Supported Protocol
 */
enum class SupportedProtocol : uint8_t {
    /**
     * @brief NMEA0183
     */
    NMEA0183,

    /**
     * @brief Ubx Binary Protocol
     */
    UbxBinaryProtocol,

    /**
     * @brief Sirf Binary Protocol
     */
    SirfBinaryProtocol,

    /**
     * @brief Fake Receiver Protocol
     */
    FakeReceiverProtocol,

    /**
     * @brief Unknown Protocol
     */
    UnknownProtocol,
};

/**
 * @brief Vendor Id
 *
 */
enum class VendorId : uint16_t {
    /**
     * @brief SiRF
     */
    SiRF = 0x067b,

    /**
     * @brief Garmin
     */
    Garmin = 0x091E,

    /**
     * @brief Ublox
     */
    Ublox = 0x1546,

    /**
     * @brief Unknown
     */
    Unknown,
};

/**
 * @brief convert int to Vendor Id
 *
 * @param value int value
 * @return VendorId
 */
VendorId toVendorId(const uint16_t value);

/**
 * @brief SW Version
 */
enum SWVersion : uint16_t {
    /**
     * @brief SPG_100
     */
    SPG_100 = 0,

    /**
     * @brief SPG_201
     */
    SPG_201 = 1,

    /**
     * @brief SPG_301
     */
    SPG_301 = 2,

    /**
     * @brief HPG_140
     */
    HPG_140,

    /**
     * @brief Unknown
     */
    TIM_110,

    /**
     * @brief Unknown
     */
    Unknown,
};

/**
 * @brief Features
 *
 */
enum class Feature : uint32_t {
    /**
     * @brief ttyDevice
     */
    ttyDevice = 1,
};

/**
 * @brief min BaudRate
 */
const uint32_t minBaudRate = 4800;

/**
 * @brief max BaudRate
 *
 */
const uint32_t maxBaudRate = 460800;

/**
 * @brief IGnssReceiver
 */
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

    /*!
     * \brief GetTransport
     * \return
     */
    virtual std::shared_ptr<Transport> GetTransport() = 0;

    /*!
     * \brief HasFeature
     * \return
     */
    virtual bool HasFeature(const Feature ftr) const = 0;

protected:
    /*!
     * \brief SetFeature
     * \return
     */
    virtual void SetFeature(const Feature ftr) = 0;
};

} // namespace android::hardware::gnss::V2_1::renesas

#endif // IGNSSRECEIVER_H
