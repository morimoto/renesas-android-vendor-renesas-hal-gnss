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
#include <GnssVisibilityControl.h>

namespace android::hardware::gnss::V2_1::renesas {
/**
 * @brief GnssCallback_1_0
 */
using GnssCallback_1_0
    = ::android::sp<::android::hardware::gnss::V1_0::IGnssCallback>;

/**
 * @brief GnssCallback_1_1
 */
using GnssCallback_1_1
    = ::android::sp<::android::hardware::gnss::V1_1::IGnssCallback>;

/**
 * @brief GnssCallback_2_0
 */
using GnssCallback_2_0
    = ::android::sp<::android::hardware::gnss::V2_0::IGnssCallback>;

/**
 * @brief GnssCallback_2_1
 */
using GnssCallback_2_1
    = ::android::sp<::android::hardware::gnss::V2_1::IGnssCallback>;

/**
 * @brief GnssVisibilityControlV1_0
 */
using GnssVisibilityControlV1_0 =
                    ::android::hardware::gnss::visibility_control::V1_0
                            ::renesas::GnssVisibilityControl;

/**
 * @brief LPError
 *
 */
enum class LPError : uint8_t {
    /**
     * @brief SUCCESS
     */
    SUCCESS,

    /**
     * @brief FAIL
     */
    FAIL,

    /**
     * @brief INTERNAL_ERROR
     */
    INTERNAL_ERROR,
};

/**
 * @brief ILocationProvider
 */
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

    /**
     * @brief Set the Callback 1.0
     *
     * @param cb
     */
    virtual void setCallback_1_0(GnssCallback_1_0& cb) = 0;

    /**
     * @brief Set the Callback 1.1
     *
     * @param cb
     */
    virtual void setCallback_1_1(GnssCallback_1_1& cb) = 0;

    /**
     * @brief Set the Callback 2.0
     *
     * @param cb
     */
    virtual void setCallback_2_0(GnssCallback_2_0& cb) = 0;

    /**
     * @brief Set the Callback 2.1
     *
     * @param cb
     */
    virtual void setCallback_2_1(GnssCallback_2_1& cb) = 0;

    /**
     * @brief Set the Update Interval
     *
     * @param newInterval
     */
    virtual void SetUpdateInterval(uint32_t newInterval) = 0;

    /**
     * @brief Set the Enabled
     *
     * @param isEnabled
     */
    virtual void SetEnabled(bool isEnabled) = 0;

    /**
     * @brief Set the Gnss Visibility Control
     *
     * @param gnssVisibilityControl
     */
    virtual void setGnssVisibilityControl(
        sp<GnssVisibilityControlV1_0>& gnssVisibilityControl) = 0;

};

} // namespace android::hardware::gnss::V2_1::renesas

#endif // ILOCATIONPROVIDER_H
