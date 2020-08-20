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

#ifndef GNSSDEBUG_H
#define GNSSDEBUG_H

#include <android/hardware/gnss/2.0/IGnssDebug.h>

namespace android::hardware::gnss::V2_1::renesas {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct GnssDebug : public android::hardware::gnss::V2_0::IGnssDebug {
    // Methods from ::android::hardware::gnss::V1_0::IGnssDebug follow.
    Return<void> getDebugData(getDebugData_cb _hidl_cb) override;

    // Methods from ::android::hardware::gnss::V2_0::IGnssDebug follow.
    Return<void> getDebugData_2_0(getDebugData_2_0_cb _hidl_cb) override;
};

}  // namespace android::hardware::gnss::V2_1::renesas

#endif // GNSSDEBUG_H
