#define LOG_TAG "GnssRenesasHAL"
#define LOG_NDEBUG 1

#include <log/log.h>
#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>
#include <android-base/logging.h>
#include <memory>

#include "include/Gnss.h"
#include "include/GeneralManager.h"

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
