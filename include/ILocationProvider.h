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

#ifndef ILOCATIONPROVIDER_H
#define ILOCATIONPROVIDER_H

#include <android/hardware/gnss/2.0/IGnssCallback.h>
#include <android/hardware/gnss/2.1/IGnssCallback.h>

namespace android::hardware::gnss::V2_1::renesas {
using GnssCallback_1_0
    = ::android::sp<::android::hardware::gnss::V1_0::IGnssCallback>;
using GnssCallback_1_1
    = ::android::sp<::android::hardware::gnss::V1_1::IGnssCallback>;
using GnssCallback_2_0
    = ::android::sp<::android::hardware::gnss::V2_0::IGnssCallback>;
using GnssCallback_2_1
    = ::android::sp<::android::hardware::gnss::V2_1::IGnssCallback>;

enum class LPError : uint8_t {
    SUCCESS,
    FAIL,
    INTERNAL_ERROR,
};


class ILocationProvider {
public:
    /*!
     * \brief ~ILocationProvider
     */
    virtual ~ILocationProvider() = default;

    /*!
     * \brief StartProviding
     */
    virtual LPError StartProviding() = 0;
    /*!
     * \brief StopProviding
     */
    virtual LPError StopProviding() = 0;

    virtual void setCallback_1_0(GnssCallback_1_0& cb) = 0;

    virtual void setCallback_1_1(GnssCallback_1_1& cb) = 0;

    virtual void setCallback_2_0(GnssCallback_2_0& cb) = 0;

    virtual void setCallback_2_1(GnssCallback_2_1& cb) = 0;

    virtual void SetUpdateInterval(uint32_t newInterval) = 0;

    virtual void SetEnabled(bool isEnabled) = 0;
};

} // namespace android::hardware::gnss::V2_1::renesas

#endif // ILOCATIONPROVIDER_H
