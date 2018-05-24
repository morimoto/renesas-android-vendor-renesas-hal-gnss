

#define LOG_TAG "GnssSalvatorHAL"

#include <android-base/logging.h>
#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>

#include "Gnss.h"

using namespace android::hardware;
using namespace android::hardware::gnss::V1_0;

int main(void) {
    android::ProcessState::initWithDriver("/dev/vndbinder");
    android::sp<IGnss> gnss_hal = new salvator::Gnss();

    configureRpcThreadpool(1, true);

    auto status = gnss_hal->registerAsService();
    CHECK_EQ(status, android::OK) << "Failed to register GNSS HAL";

    joinRpcThreadpool();
}
