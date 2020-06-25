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

#pragma once

#include <android/hardware/gnss/2.0/IGnssBatching.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace android::hardware::gnss::V2_0::renesas {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct GnssBatching : public IGnssBatching {
    // Methods from ::android::hardware::gnss::V1_0::IGnssBatching follow.
    Return<bool> init(const
        sp<::android::hardware::gnss::V1_0::IGnssBatchingCallback>&
        callback) override;
    Return<uint16_t> getBatchSize() override;
    Return<bool> start(
        const ::android::hardware::gnss::V1_0::IGnssBatching::Options& options)
    override;
    Return<void> flush() override;
    Return<bool> stop() override;
    Return<void> cleanup() override;

    // Methods from ::android::hardware::gnss::V2_0::IGnssBatching follow.
    Return<bool> init_2_0(const
        sp<::android::hardware::gnss::V2_0::IGnssBatchingCallback>&
        callback) override;


};

}  // namespace android::hardware::gnss::V2_0::renesas
