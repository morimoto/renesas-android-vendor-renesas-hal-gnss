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

#include <cmath>

#include "include/IGnssReceiver.h"

enum class ProductId : uint16_t {
    Ublox4 = 0x01a4,
    Ublox5 = 0x01a5,
    Ublox6 = 0x01a6,
    Ublox7 = 0x01a7,
    Ublox8 = 0x01a8,
};

class UbloxReceiver : public IGnssReceiver {
public:
    /*!
     * \brief UbloxReceiver
     * \param vendorId
     * \param productId
     */
    UbloxReceiver(uint16_t vendorId, uint16_t productId,
                  const GnssReceiverType& type);

    /*!
     * \brief UbloxReceiver
     * \param vendorId
     * \param productId
     * \param path
     */
    UbloxReceiver(uint16_t vendorId, uint16_t productId, std::string& path,
                  const GnssReceiverType& type);

    UbloxReceiver(const std::string& path, const GnssReceiverType& type);

    /*!
     * \brief UbloxReceiver
     */
    ~UbloxReceiver() override;

    /*!
     * \brief GetReceiverType
     * \return
     */
    GnssReceiverType GetReceiverType() override;

    /*!
     * \brief GetVendorId
     * \return
     */
    GnssVendor GetVendorId() override;

    /*!
     * \brief GetProductId
     * \return
     */
    GnssProduct GetProductId() override;

    /*!
     * \brief GetYearOfHw
     * \return
     */
    uint32_t GetYearOfHw() override;

    /*!
     * \brief GetFirmwareVersionStr
     * \return
     */
    RError GetFirmwareVersionStr(std::string& out) override;

    /*!
     * \brief GetFirmwareVersion
     * \return
     */
    float GetFirmwareVersion() override;

    /*!
     * \brief GetVendorName
     * \return
     */
    RError GetVendorName(std::string& out) override;

    /*!
     * \brief GetProductName
     * \return
     */
    RError GetProductName(std::string& out) override;

    /*!
     * \brief GetPath
     * \return
     */
    RError GetPath(std::string& out) override;

    /*!
     * \brief GetSupportedProtocols
     */
    void GetSupportedProtocols(std::vector<SupportedProtocol>& out) override;

    /*!
     * \brief GetBaudRate
     * \return
     */
    uint32_t GetBaudRate() override;

    /*!
    * \brief SetVendor
    * \return
    */
    RError SetVendor();

    template <typename T>
    RError GetUbxProtocolVersion(T& out);

    RError SetBaudRate(const uint32_t& baudrate) override;
    RError SetFwVersion(const double& version) override;
    RError SetSecondMajorConstellation(const std::string&
                                        constellation) override;
    RError SetSbasStatus(const std::string& status) override;
    SWVersion GetSwVersion() override;

protected:
    /*!
     * \brief UbloxReceiver
     */
    UbloxReceiver() = default;

    /*!
     * \brief SetSupportedProtocols
     */
    void SetSupportedProtocols();

private:
    UbloxReceiver& operator=(const UbloxReceiver&) = delete;
    UbloxReceiver(const UbloxReceiver&) = delete;

    const uint32_t defaultLineBaudRate = 9600u;
    const double SPG_100 = 1.00;
    const double SPG_201 = 2.01;
    const double SPG_301 = 3.01;

    GnssVendor mVendor = GnssVendor::Ublox;
    GnssProduct mProduct;
    GnssReceiverType mReceiverType;

    uint16_t mVendorId = static_cast<uint16_t>(VendorId::Ublox);
    uint16_t mProductId;

    uint32_t mLineBaudRate = defaultLineBaudRate;
    uint32_t mYearOfHw = 0u;
    float mFirmware = 0.0f;


    std::string mVendorName = "u-blox";
    std::string mProductName;
    std::string mTtyPath;
    std::string mFirmwareVersion;

    std::string mSecondMajorConstellation = "glonass";
    std::string mSbasStatus = "enabled";
    bool mSbasEnabled = true;

    std::string mUbloxProtocolMaxVersion;
    uint32_t mUbloxPrVersionMax;

    SWVersion mSwVersion = SWVersion::Unknown;
    std::vector<SupportedProtocol> mProtocolList;
};
