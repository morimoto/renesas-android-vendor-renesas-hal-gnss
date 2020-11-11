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

#ifndef GNSSBATCHING_H
#define GNSSBATCHING_H

#include <android/hardware/gnss/2.0/IGnssBatching.h>

namespace android::hardware::gnss::V2_1::renesas {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

/**
 * @brief GnssBatching
 */
struct GnssBatching : public android::hardware::gnss::V2_0::IGnssBatching {
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

}  // namespace android::hardware::gnss::V2_1::renesas

#endif // GNSSBATCHING_H
