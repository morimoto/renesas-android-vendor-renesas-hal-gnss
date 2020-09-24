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
#define LOG_TAG "GnssRenesasVisibilityControl"
#define LOG_NDEBUG 1

#include "include/GnssVisibilityControl.h"

#include <log/log.h>
#include <utils/SystemClock.h>

static const int updateInterval = 1000; //1s
static const std::string requestorId = {"Renesas"};

namespace android {
namespace hardware {
namespace gnss {
namespace visibility_control {
namespace V1_0 {
namespace renesas {

using NfwProtocolStack =
    ::android::hardware::gnss::visibility_control::V1_0
        ::IGnssVisibilityControlCallback::NfwProtocolStack;
using NfwRequestor =
    ::android::hardware::gnss::visibility_control::V1_0
        ::IGnssVisibilityControlCallback::NfwRequestor;
using NfwResponseType =
    ::android::hardware::gnss::visibility_control::V1_0
        ::IGnssVisibilityControlCallback::NfwResponseType;

Return<bool> GnssVisibilityControl::enableNfwLocationAccess(
    const hidl_vec<hidl_string>& proxyApps) {
    mNfwEnable = (0 == proxyApps.size()) ? false : true;

    if (mNfwEnable) {
        mProxyApps = proxyApps;
    }

    return true;
}

void GnssVisibilityControl::sendNfwNotificationMsg(const std::string& pkgName){
    ALOGV("%s", __func__);
    int64_t uptimeMillis = ::android::elapsedRealtime();

    if(mNfwEnable && uptimeMillis - mLastSend > updateInterval){
        bool isInEmergencySession = mGnssVisibilityControlCbIface->isInEmergencySession();
        NfwNotification msg = {
            .proxyAppPackageName = isInEmergencySession ?
                hidl_string{} : hidl_string{pkgName},
            .protocolStack = NfwProtocolStack::SUPL,
            .otherProtocolStackName = {},
            .requestor = NfwRequestor::AUTOMOBILE_CLIENT,
            .requestorId = {requestorId},
            .responseType = NfwResponseType::ACCEPTED_LOCATION_PROVIDED,
            .inEmergencyMode = isInEmergencySession,
            .isCachedLocation = false,
        };

        if (nullptr != mGnssVisibilityControlCbIface) {
            mGnssVisibilityControlCbIface->nfwNotifyCb(msg);
        }

        mLastSend = uptimeMillis;
    }
}
Return<bool> GnssVisibilityControl::setCallback(
    const sp<V1_0::IGnssVisibilityControlCallback>& callback) {
    mGnssVisibilityControlCbIface = callback;
    return true;
}

}  // namespace renesas
}  // namespace V1_0
}  // namespace visibility_control
}  // namespace gnss
}  // namespace hardware
}  // namespace android
