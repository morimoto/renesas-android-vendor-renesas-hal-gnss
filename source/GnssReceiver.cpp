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

#include "include/GnssReceiver.h"

namespace android::hardware::gnss::V2_1::renesas {

GnssReceiver::GnssReceiver(Transport* transport) : mTransport(transport) {}

std::shared_ptr<Transport> GnssReceiver::GetTransport() {
    return mTransport;
}

bool GnssReceiver::HasFeature(const Feature ftr) const {
    return (static_cast<u_int32_t>(ftr) & mFeatures) != 0;
}

void GnssReceiver::SetFeature(const Feature ftr) {
    mFeatures |= static_cast<u_int32_t>(ftr);
}

}  // namespace android::hardware::gnss::V2_1::renesas
