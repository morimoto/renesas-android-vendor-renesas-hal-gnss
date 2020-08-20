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

#include "include/IGnssReceiver.h"

namespace android::hardware::gnss::V2_1::renesas {

class GnssReceiver : public IGnssReceiver {
public:
    GnssReceiver(Transport* transport);
    virtual std::shared_ptr<Transport> GetTransport() override;
    bool HasFeature(const Feature ftr) const;
protected:
    void SetFeature(const Feature ftr) override;
private:
    GnssReceiver(const GnssReceiver&) = delete;
    std::shared_ptr<Transport> mTransport;
    uint32_t mFeatures = 0;
};

} // namespace android::hardware::gnss::V2_1::renesas

#endif // GNSSRECEIVER_H
