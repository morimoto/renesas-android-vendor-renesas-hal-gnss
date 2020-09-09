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

#include "include/NmeaGsa.h"

template<>
NPError NmeaGsa<SvInfoFixOutType>::GetData(SvInfoFixOutType out) {
    out.gnssId = mParcel.gnssId;
    out.svList = mParcel.svList;

    return NPError::Success;
}

size_t talkerIdToGnssId(std::string talkerId) {
    size_t firstCharIndex = 0;

    if ('$' == talkerId[firstCharIndex]) {
        talkerId = talkerId.erase(firstCharIndex, 1);
    }

    for (size_t gnssId = 0; gnssId < sizeof(NmeaCallerIdGnssIdPair) /
                                         sizeof(NmeaCallerIdGnssIdPair[0]);
         ++gnssId) {
        if (0 == talkerId.compare(firstCharIndex,
                                  2,  // only two character to compare
                                  NmeaCallerIdGnssIdPair[gnssId].first)) {
            return NmeaCallerIdGnssIdPair[gnssId].second;
        }
    }

    return 0;
}
