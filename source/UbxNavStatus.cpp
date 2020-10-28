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

#include <UbxNavStatus.h>

using GnssData =
    android::hardware::gnss::V2_1::IGnssMeasurementCallback::GnssData;

template <>
UPError UbxNavStatus<GnssData*>::GetData(GnssData* out) {
    if (mIsValid) {
        out->clock.v1_0.fullBiasNs -= (out->clock.v1_0.timeNs - mTimeNano);
        out->clock.v1_0.timeNs = mTimeNano;
        return UPError::Success;
    }

    return UPError::InvalidData;
}
