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

#include "include/NmeaGsv.h"

using GnssSvInfo
    = ::android::hardware::gnss::V2_0::IGnssCallback::GnssSvInfo;
using GnssConstellationType1_0
    = ::android::hardware::gnss::V1_0::GnssConstellationType;
using GnssConstellationType2_0
    = ::android::hardware::gnss::V2_0::GnssConstellationType;

static GnssConstellationType1_0 Constallation_2_0_to_1_0(
        const GnssConstellationType2_0 &constallation) {
    GnssConstellationType1_0 result = GnssConstellationType1_0::UNKNOWN;

    switch (constallation) {
        case GnssConstellationType2_0::GPS:
            result = GnssConstellationType1_0::GPS;
            break;

        case GnssConstellationType2_0::SBAS:
            result = GnssConstellationType1_0::SBAS;
            break;

        case GnssConstellationType2_0::GLONASS:
            result = GnssConstellationType1_0::GLONASS;
            break;

        case GnssConstellationType2_0::QZSS:
            result = GnssConstellationType1_0::QZSS;
            break;

        case GnssConstellationType2_0::BEIDOU:
            result = GnssConstellationType1_0::BEIDOU;
            break;

        case GnssConstellationType2_0::GALILEO:
            result = GnssConstellationType1_0::GALILEO;
            break;

        default:
            break;
    }

    return result;
}

template<>
NPError NmeaGsv<SvInfoOutType>::GetData(SvInfoOutType out) {
    if (!mIsValid) {
        return NPError::InvalidData;
    }

    out.gnssId = mParcel.gnssId;
    out.msgAmount = mParcel.msgAmount;
    out.msgNum = mParcel.curMsgNum;

    for (auto gsv : mParcel.subPart) {
        GnssSvInfo sv;
        sv.v1_0.svid = gsv.svid;
        sv.v1_0.cN0Dbhz = gsv.cn0;
        sv.v1_0.elevationDegrees = gsv.elevation;
        sv.v1_0.azimuthDegrees = gsv.azimuth;
        sv.v1_0.carrierFrequencyHz = mParcel.carrierFrequencyHz;
        sv.v1_0.svFlag = mParcel.svFlag;
        sv.v1_0.constellation = Constallation_2_0_to_1_0(gsv.constellation);
        sv.constellation = gsv.constellation;
        out.svInfoList.push_back(sv);
    }

    return NPError::Success;
}
