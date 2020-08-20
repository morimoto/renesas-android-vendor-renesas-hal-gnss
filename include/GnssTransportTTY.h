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

#ifndef GNSSTRANSPORTTTY_H
#define GNSSTRANSPORTTTY_H

#include "include/GnssTransport.h"

namespace android::hardware::gnss::V2_1::renesas {

class GnssTransportTTY: public Transport {
public:
    GnssTransportTTY(const std::string &filePath);

     /*!
     * \brief GnssTransportTTY - destructor
     */
    virtual ~GnssTransportTTY();

    /*!
     * \brief GetBaudRate
     * \return
     */
    virtual uint32_t GetBaudRate() const {
        return mBaudRate;
    };

    /*!
     * \brief SetBaudRate
     * \param baudrate
     * \return
     */
    virtual TError SetBaudRate(const uint32_t& baudrate);

    /*!
     * \brief Allow to reset GNSS-receiver power
     *
     * \param allowReset If true receiver will be reset
     */
    void SetOpenReceiverWithReset(const bool allowReset) {
        mAllowReset = allowReset;
    }
protected:
    TError SetUp() override;
    TError Open() override;
    TError Close() override;
    TError WriteData(const std::vector<uint8_t> &toWrite) override;
    virtual TError ResetReceiver();
    char ReadByte(TError& errCode) override;
private:
    bool mAllowReset = false;
    uint32_t mBaudRate;
    int mFd;
};

} // namespace android::hardware::gnss::V2_1::renesas

#endif // GNSSTRANSPORTTTY_H
