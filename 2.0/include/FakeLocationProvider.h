/*
 * Copyright (C) 2019 GlobalLogic
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

#include "include/LocationProviderBase.h"
#include "include/FakeLocationBuilder.h"

namespace android::hardware::gnss::V2_0::renesas {

class FakeLocationProvider : public LocationProviderBase {
public:
    FakeLocationProvider(uint32_t interval);
    virtual ~FakeLocationProvider() override = default;
protected:
    void Provide() override;
private:
    FakeLocationProvider(FakeLocationProvider&) = delete;
    FakeLocationProvider& operator=(const FakeLocationProvider&) = delete;

    std::unique_ptr<FakeLocationBuilder> mBuilder;
};

} // namespace android::hardware::gnss::V2_0::renesas
