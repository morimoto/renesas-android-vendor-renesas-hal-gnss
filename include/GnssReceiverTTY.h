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
#ifndef GNSSRECEIVERTTY_H
#define GNSSRECEIVERTTY_H

#include "GnssReceiver.h"
#include "GnssTransportTTY.h"

namespace android::hardware::gnss::V2_1::renesas {

class GnssReceiverTTY : public GnssReceiver {
public:
    GnssReceiverTTY(GnssTransportTTY* transport)
        : GnssReceiver(transport) {}

    /*!
     * \brief GetBaudRate
     * \return
     */
    virtual uint32_t GetBaudRate() = 0;

    /*!
     * \brief SetBaudRate
     * \param baudrate
     * \return
     */
    virtual RError SetBaudRate(const uint32_t& baudrate) = 0;

    /*!
     * \brief Get transport TTY interface
     * \return TTY interface
     */
    std::shared_ptr<GnssTransportTTY> GetTransportTTY() {
        return std::static_pointer_cast<GnssTransportTTY>(GetTransport());
    }
};

} // namespace android::hardware::gnss::V2_0::renesas

#endif // GNSSRECEIVERTTY_H
