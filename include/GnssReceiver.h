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

#ifndef GNSSRECEIVER_H
#define GNSSRECEIVER_H

#include <IGnssReceiver.h>

namespace android::hardware::gnss::V2_1::renesas {

/**
 * @brief Gnss Receiver class
 *
 */
class GnssReceiver : public IGnssReceiver {
public:
    /**
     * @brief Construct a new Gnss Receiver object
     *
     * @param transport
     */
    GnssReceiver(Transport* transport);

    /**
     * @brief Get the Transport object
     *
     * @return std::shared_ptr<Transport>
     */
    virtual std::shared_ptr<Transport> GetTransport() override;

    /**
     * @brief HasFeature
     *
     * @param ftr feature
     * @return true
     * @return false
     */
    bool HasFeature(const Feature ftr) const;
protected:
    /**
     * @brief Set the Feature object
     *
     * @param ftr feature
     */
    void SetFeature(const Feature ftr) override;
private:
    GnssReceiver(const GnssReceiver&) = delete;
    std::shared_ptr<Transport> mTransport;
    uint32_t mFeatures = 0;
};

} // namespace android::hardware::gnss::V2_1::renesas

#endif // GNSSRECEIVER_H
