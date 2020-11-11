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

#ifndef LOCATIONPROVIDERBASE_H
#define LOCATIONPROVIDERBASE_H

#include <atomic>
#include <thread>
#include <unordered_map>

#include <ILocationProvider.h>

namespace android::hardware::gnss::V2_1::renesas {

/**
 * @brief GnssCallback_1_0
 */
using GnssCallback_1_0 = android::sp<android::hardware::gnss::V1_0::IGnssCallback>;

/**
 * @brief GnssCallback_1_1
 */
using GnssCallback_1_1 = android::sp<android::hardware::gnss::V1_1::IGnssCallback>;

/**
 * @brief GnssCallback_2_0
 */
using GnssCallback_2_0 = android::sp<android::hardware::gnss::V2_0::IGnssCallback>;

/**
 * @brief GnssCallback_2_1
 */
using GnssCallback_2_1 = android::sp<android::hardware::gnss::V2_1::IGnssCallback>;

/**
 * @brief Location Data
 */
using LocationData     = android::hardware::gnss::V2_0::GnssLocation;

/**
 * @brief Location Provider Base
 */
class LocationProviderBase : public ILocationProvider {
public:
    /**
     * @brief Construct a new Location Provider Base object
     *
     * @param interval
     */
    LocationProviderBase(uint32_t interval);

    /**
     * @brief Destroy the Location Provider Base object
     */
    ~LocationProviderBase() override;

    /**
     * @brief Start Providing
     *
     * @return LPError
     */
    LPError StartProviding() override;

    /**
     * @brief Stop Providing
     *
     * @return LPError
     */
    LPError StopProviding() override;

    /**
     * @brief Set the Update Interval object
     *
     * @param newInterval
     */
    void SetUpdateInterval(uint32_t newInterval) override;

    /**
     * @brief Set the Callback 1.0 object
     *
     * @param cb
     */
    void setCallback_1_0(GnssCallback_1_0& cb) override;

    /**
     * @brief Set the Callback 1.1 object
     *
     * @param cb
     */
    void setCallback_1_1(GnssCallback_1_1& cb) override;

    /**
     * @brief Set the Callback 2.0 object
     *
     * @param cb
     */
    void setCallback_2_0(GnssCallback_2_0& cb) override;

    /**
     * @brief Set the Callback 2.1 object
     *
     * @param cb
     */
    void setCallback_2_1(GnssCallback_2_1& cb) override;

    /**
     * @brief Set the Enabled
     *
     * @param isEnabled
     */
    void SetEnabled(bool isEnabled) override;

    /**
     * @brief Set the Gnss Visibility Control object
     *
     * @param gnssVisibilityControl
     */
    void setGnssVisibilityControl(
        sp<GnssVisibilityControlV1_0>& gnssVisibilityControl) override;

protected:
    /*!
     * \brief Provide
     */
    virtual void Provide() = 0;

    /**
     * @brief Call Providers
     *
     * @param location
     */
    void CallProviders(const LocationData& location);

    /**
     * @brief mThreadExit
     */
    std::atomic<bool> mThreadExit;

    /**
     * @brief mGnssLocationThread
     */
    std::thread mGnssLocationThread;

    /**
     * @brief GnssCallback 1.0
     */
    GnssCallback_1_0 mGnssCallback_1_0;

    /**
     * @brief GnssCallback 1.1
     */
    GnssCallback_1_1 mGnssCallback_1_1;

    /**
     * @brief GnssCallback 2.0
     */
    GnssCallback_2_0 mGnssCallback_2_0;

    /**
     * @brief GnssCallback 2.1
     */
    GnssCallback_2_1 mGnssCallback_2_1;

    /**
     * @brief mEnabled
     */
    std::atomic<bool> mEnabled;

    /**
     * @brief mUpdateIntervalUs
     */
    uint32_t mUpdateIntervalUs;

    /**
     * @brief mGnssVisibilityControl
     */
    sp<GnssVisibilityControlV1_0> mGnssVisibilityControl;
private:
    LocationProviderBase(LocationProviderBase&) = delete;
    LocationProviderBase& operator=(const LocationProviderBase&) = delete;
    std::unordered_map<std::string, std::function<void(const LocationData&)>> mProviders;
};

} // namespace android::hardware::gnss::V2_1::renesas

#endif // LOCATIONPROVIDERBASE_H
