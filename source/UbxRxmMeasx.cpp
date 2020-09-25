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

#define LOG_TAG "GnssRenesasUbxRxmMeasxImpl"

#include "include/UbxRxmMeasx.h"

using GnssData =
    android::hardware::gnss::V2_1::IGnssMeasurementCallback::GnssData;

static constexpr double pseudorangeRateScaleUp = 0.04;

template<>
float UbxRxmMeasx<GnssData*>::GetCarrierFrequencyFromGnssId(
    const UbxGnssId inGnssId) {
    const float L1BandFrequency = 1575.42f;
    const float B1BandFrequency = 1561.098f;
    const float L1GlonassBandFrequency = 1602.562f;
    const float scale = 1000000.0;

    switch (inGnssId) {
    case UbxGnssId::GPS:
    case UbxGnssId::SBAS:
    case UbxGnssId::GALILEO:
    case UbxGnssId::QZSS:
        return ScaleUp(L1BandFrequency, scale);

    case UbxGnssId::BEIDOU:
        return ScaleUp(B1BandFrequency, scale);

    case UbxGnssId::GLONASS:
        return ScaleUp(L1GlonassBandFrequency, scale);

    default:
        return 0;
    }
}

template<>
UPError UbxRxmMeasx<GnssData*>::GetGnssMeasurement(
    repeatedBlock_t& block, GnssMeasx& measurement) {
    auto& meas_v1_0 = measurement.v2_0.v1_1.v1_0;
    UbxGnssId id = static_cast<UbxGnssId>(block.gnssId);
    meas_v1_0.carrierFrequencyHz = GetCarrierFrequencyFromGnssId(
                                       static_cast<UbxGnssId>
                                       (block.gnssId));
    meas_v1_0.flags = 0;

    if (meas_v1_0.carrierFrequencyHz > 0.0f) {
        meas_v1_0.flags = static_cast<uint32_t>
                          (GnssMF::HAS_CARRIER_FREQUENCY);
    }

    meas_v1_0.svid = GetValidSvidForGnssId(id, block.svId);
    measurement.v2_0.codeType = "C";
    measurement.v2_0.constellation = GetConstellationFromGnssId(id);
    GetTowForGnssId(id, meas_v1_0.receivedSvTimeInNs, measurement.v2_0.state);
    GetTowAccForGnssId(id, meas_v1_0.receivedSvTimeUncertaintyInNs,
                       measurement.v2_0.state);
    meas_v1_0.cN0DbHz = static_cast<double>(block.cn0);
    meas_v1_0.multipathIndicator = (block.multipath == 0) ?
                                   GnssMI::INDICATIOR_NOT_PRESENT :
                                   GnssMI::INDICATOR_PRESENT;
    meas_v1_0.pseudorangeRateMps = ScaleUp(block.pseudoRangeRate,
                                           pseudorangeRateScaleUp);
    meas_v1_0.pseudorangeRateUncertaintyMps =
        0.075; // TODO: set real value, at moment it is pure magic.
    meas_v1_0.accumulatedDeltaRangeState = static_cast<uint16_t>
                                           (GnssADRS::ADR_STATE_UNKNOWN);
    return UPError::Success;
}

template <>
UPError UbxRxmMeasx<GnssData*>::GetData(GnssData* out) {
    if (!mIsValid) {
        return UPError::InvalidData;
    }

    std::vector<GnssMeasx> measx;

    for (auto& block : mParcel.repeated) {
        GnssMeasx measurement = {};

        if (auto err = GetGnssMeasurement(block, measurement);
            err != UPError::Success) {
            return err;
        }

        measx.push_back(measurement);
    }

    out->measurements = android::hardware::hidl_vec(measx);
    return UPError::Success;
}
