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

using GnssSvInfo = ::android::hardware::gnss::V2_0::IGnssCallback::GnssSvInfo;

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
        sv.constellation = gsv.constellation;
        out.svInfoList.push_back(sv);
    }

    return NPError::Success;
}
