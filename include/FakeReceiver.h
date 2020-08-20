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

#ifndef FAKERECEIVER_H
#define FAKERECEIVER_H

#include "include/GnssReceiver.h"

namespace android::hardware::gnss::V2_1::renesas {

class FakeReceiver : public GnssReceiver {
public:
    /*!
     * \brief FakeReceiver
     * \param path
     */
    FakeReceiver(const std::string& path);

    /*!
     * \brief ~FakeReceiver
     */
    ~FakeReceiver() override = default;

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
     */
    void GetSupportedProtocols(std::vector<SupportedProtocol>&) override;

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
     * \brief FakeReceiver
     */
    FakeReceiver() = default;

private:
    FakeReceiver& operator=(const FakeReceiver&) = delete;
    FakeReceiver(const FakeReceiver&) = delete;
    GnssVendor mVendor = GnssVendor::Fake;
    GnssProduct mProduct  = GnssProduct::Fake;
    GnssReceiverType mReceiverType = GnssReceiverType::FakeReceiver;
    SupportedProtocol mProtocol = SupportedProtocol::FakeReceiverProtocol;


    uint32_t mYearOfHw = 0;
    float mFirmware = 0.0f;
    SWVersion mSwVersion = SWVersion::Unknown;

    std::string mVendorName = "Fake";
    std::string mProductName = "Fake";
    std::string mFirmwareVersion = "0.0";
};

} //namespace android::hardware::gnss::V2_1::renesas

#endif // FAKERECEIVER_H
