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

#ifndef __GNSSIPARSER_H__
#define __GNSSIPARSER_H__

#include <android/hardware/gnss/1.0/types.h>
#include <android/hardware/gnss/1.0/IGnssMeasurementCallback.h>

using ::android::hardware::gnss::V1_0::IGnssMeasurementCallback;
using ::android::hardware::gnss::V1_0::GnssConstellationType;

class GnssIParser {
public:
    enum GnssIParserReturn : uint8_t {
        NotReady = 0,
        RxmDone = 1,
        GPSTimeDone = 1 << 1,
        ClockDone = 1 << 2,
        StatusDone = 1 << 3,
        Ready = (RxmDone | GPSTimeDone | ClockDone | StatusDone),
        UTCDone, // move up when needed
    };

    /*!
     * \brief retrieveSvInfo - provide data collected from parsed messages
     * \param gnssData - a reference to fill the object
     * \return GnssIParserReturn::*Done on success, otherwise NotReady
     */
    virtual uint8_t retrieveSvInfo(IGnssMeasurementCallback::GnssData &gnssData) = 0;

    /*!
     * \brief dumpDebug - print some debug info or hexdump for concrete parser
     */
    virtual void dumpDebug() = 0;
    virtual ~GnssIParser();
};

#endif // __GNSSIPARSER_H__
