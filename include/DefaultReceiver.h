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

#include "include/GnssReceiverTTY.h"

namespace android::hardware::gnss::V2_0::renesas {

class DefaultReceiver : public GnssReceiverTTY {
public:
    /*!
     * \brief DefaultReceiver
     * \param path
     */
    DefaultReceiver(const std::string& path, const GnssReceiverType& type);

    /*!
     * \brief DefaultReceiver
     * \param vendorId
     * \param productId
     * \param path
     */
    DefaultReceiver(uint16_t vendorId, uint16_t productId, const std::string&
                    path, const GnssReceiverType& type);

    /*!
     * \brief ~DefaultReceiver
     */
    ~DefaultReceiver() override;

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
     * \brief GetSupportedProtocols
     * \param out
     */
    void GetSupportedProtocols(std::vector<SupportedProtocol>& out) override;

    /*!
     * \brief GetBaudRate
     * \return
     */
    uint32_t GetBaudRate() override;

    /*!
     * \brief SetBaudRate
     * \param baudrate
     * \return
     */
    RError SetBaudRate(const uint32_t& baudrate) override;

    /*!
     * \brief SetFwVersion
     * \param version
     * \return
     */
    RError SetFwVersion(const double& version) override;

    /*!
     * \brief SetSecondMajorConstellation
     * \param constellation
     * \return
     */
    RError SetSecondMajorConstellation([[maybe_unused]] const std::string&
                                       constellation) override;
    /*!
     * \brief SetSbasStatus
     * \param status
     * \return
     */
    RError SetSbasStatus([[maybe_unused]] const std::string& status) override;

    /*!
     * \brief GetSwVersion
     * \return
     */
    SWVersion GetSwVersion() override;

protected:
    /*!
     * \brief SetVendor
     * \return
     */
    RError SetVendor();

private:
    const uint32_t defaultLineBaudRate = 4800;
    DefaultReceiver& operator=(const DefaultReceiver&) = delete;
    DefaultReceiver(const DefaultReceiver&) = delete;

    GnssVendor mVendor = GnssVendor::Unknown;
    GnssProduct mProduct;
    GnssReceiverType mReceiverType;

    uint16_t mVendorId;
    uint16_t mProductId;

    uint32_t mLineBaudRate = defaultLineBaudRate;
    uint32_t mYearOfHw = 0;
    float mFirmware;
    SWVersion mSwVersion = SWVersion::Unknown;

    std::string mVendorName;
    std::string mProductName;
    std::string mTtyPath;
    std::string mFirmwareVersion;
    SupportedProtocol mProtocol = SupportedProtocol::NMEA0183;
};

} // namespace android::hardware::gnss::V2_0::renesas
