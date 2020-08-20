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

#define LOG_TAG "GnssRenesasHAL"
#define LOG_NDEBUG 1

#include <android-base/logging.h>
#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>
#include <log/log.h>

#include "include/GeneralManager.h"
#include "include/Gnss.h"

//TODO: try to use LazyServiceRegistrar registrar();

int main(void) {
    ALOGV("Start service");
    android::ProcessState::initWithDriver("/dev/vndbinder");
    android::hardware::configureRpcThreadpool(1, true);
    auto gnssHal =
        std::make_shared<android::hardware::gnss::V2_1::renesas::GnssImpl>();
    auto genManager = std::make_shared
        <android::hardware::gnss::V2_1::renesas::GeneralManager>();
    CHECK_EQ(android::OK, gnssHal->SetGeneralManager(genManager)) <<
            "Failed to register GNSS HAL";
    CHECK_EQ(android::OK, genManager->Run()) << "Failed to run GeneralManager";
    CHECK_EQ(android::OK, gnssHal->registerAsService()) <<
            "Failed to register GNSS HAL";
    ALOGV("Successfully started");
    android::hardware::joinRpcThreadpool();
    return 0;
}
