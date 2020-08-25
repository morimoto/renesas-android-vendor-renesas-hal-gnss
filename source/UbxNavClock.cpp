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
#include "include/UbxNavClock.h"

using GnssData =
    android::hardware::gnss::V2_0::IGnssMeasurementCallback::GnssData;
using GnssCF =
    android::hardware::gnss::V2_0::IGnssMeasurementCallback::GnssClockFlags;

static constexpr uint16_t gnssClockFlags = static_cast<uint16_t>
        (GnssCF::HAS_BIAS | GnssCF::HAS_BIAS_UNCERTAINTY |
         GnssCF::HAS_DRIFT | GnssCF::HAS_DRIFT_UNCERTAINTY);
static constexpr double psToNsScale = 1000.0;

template <>
UPError UbxNavClock<GnssData*>::GetData(GnssData* out) {
    if (!mIsValid) {
        return UPError::InvalidData;
    }

    out->clock.biasNs = static_cast<int64_t>(mParcel.clockBias);
    out->clock.driftNsps = static_cast<double>(mParcel.clockDrift);
    out->clock.biasUncertaintyNs = static_cast<double>(mParcel.timeAccuracy);
    out->clock.driftUncertaintyNsps = ScaleDown(mParcel.freqAccuracyEstimate,
                                      psToNsScale);
    out->clock.hwClockDiscontinuityCount = 0;
    out->clock.gnssClockFlags |= gnssClockFlags;
    return UPError::Success;
}
