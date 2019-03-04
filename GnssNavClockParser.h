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

#ifndef __GNSSNAVCLOCKPARSER_H__
#define __GNSSNAVCLOCKPARSER_H__

#include <vector>
#include <cstdint>
#include <list>
#include <queue>

#include <android/hardware/gnss/1.0/types.h>
#include <android/hardware/gnss/1.0/IGnssMeasurementCallback.h>

#include "GnssParserCommonImpl.h"

/* From u-blox 8 / u-blox M8 Receiver Description - Manual
 * 33.18.2 UBX-RXM-MEASX (0x02 0x14)
 * 33.18.2.1 Satellite Measurements for RRLP
 * Message: UBX-NAV-PVT
 * Description: Satellite Measurements for RRLP
*/

using ::android::hardware::gnss::V1_0::IGnssMeasurementCallback;
using ::android::hardware::gnss::V1_0::GnssConstellationType;

class GnssNavClockParser : public GnssParserCommonImpl {
public:
    /*!
     * \brief GnssNavClockParser
     * \param payload - a pointer to the payload of incoming message
     * \param payloadLen - length in bytes of payload
     */
    GnssNavClockParser(const char* payload, uint16_t payloadLen);
    ~GnssNavClockParser(){}

    /*!
     * \brief retrieveSvInfo - fill the gnssData object with collected data
     * \param gnssData - reference to gnssData object
     * \return  ClockDone on success, otherwise NotReady
     */
    uint8_t retrieveSvInfo(IGnssMeasurementCallback::GnssData &gnssData) final;

    /*!
     * \brief dumpDebug - print log in logcat, and write dump to file
     */
    void dumpDebug() override;

protected:
    GnssNavClockParser(){}

    /*!
     * \brief parseNavClockMsg
     */
    void parseNavClockMsg();

    /*!
     * \brief parseSingleBlock - parser incoming message of type ubx-nav-clock
     * \brief get clock bias, clock drift, time accuracy and frequency estimate
     */
    void parseSingleBlock();

    /*!
     * \brief getGnssClock - fill the clock member, of GnssData struct
     * \param instance - a reference to the clockfield of GnssData structure
     */
    void getGnssClock(IGnssMeasurementCallback::GnssClock &instance);

private:
    typedef struct NavClock {
        uint32_t iTow;
        int32_t clockBias;
        int32_t clockDrift;
        uint32_t timeAccuracy;
        uint32_t freqAccuracyEstimate;
    } navClock_t;

    uint8_t* mPayload = nullptr;
    uint16_t mPayloadLen = 0;

    bool mValid = false;
    navClock_t data;
};

#endif //__GNSSNAVCLOCKPARSER_H__
