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

#ifndef __GNSSNAVTIMEGPSPARSER_H__
#define __GNSSNAVTIMEGPSPARSER_H__

#include <vector>
#include <cstdint>
#include <list>
#include <queue>

#include <android/hardware/gnss/1.0/types.h>
#include <android/hardware/gnss/1.0/IGnssMeasurementCallback.h>

#include "GnssParserCommonImpl.h"

/* From u-blox 8 / u-blox M8 Receiver Description - Manual
 * 32.18.26 UBX-NAV-TIMEGPS (0x01 0x20)
 * 32.18.26.1 GPS Time Solution
 * Message: NAV-TIMEGPS
 * Description: GPS Time Solution
*/

using ::android::hardware::gnss::V1_0::IGnssMeasurementCallback;
using ::android::hardware::gnss::V1_0::GnssConstellationType;

class GnssNavTimeGPSParser : public GnssParserCommonImpl {

public:
    /*!
     * \brief GnssNavTimeGPSParser
     * \param payload - a pointer to the payload of incoming message
     * \param payloadLen - length in bytes of payload
     */
    GnssNavTimeGPSParser(const char* payload, uint16_t payloadLen);
    ~GnssNavTimeGPSParser(){}

    /*!
     * \brief retrieveSvInfo - fill the gnssData object with collected data
     * \param gnssData - reference to gnssData object
     * \return  GPSTimeDone on success, otherwise NotReady
     */
    uint8_t retrieveSvInfo(IGnssMeasurementCallback::GnssData &gnssData) final;

    /*!
     * \brief dumpDebug - print log in logcat, and write dump to file
     */
    void dumpDebug() override;

private:
    typedef struct SingleBlock {
        uint32_t iTow;
        int32_t fTow;
        int16_t week;
        int8_t leapS;
        uint8_t valid;
        uint32_t tAcc;
    } singleBlock_t;

    uint8_t* mPayload = nullptr;
    uint16_t mPayloadLen = 0;

    singleBlock_t data;
    int64_t timeNano = 0;
    bool mValid = false;

protected:
    GnssNavTimeGPSParser(){}

    /*!
     * \brief parseNavTimeGPSMsg - collect data from input message, set validity
     */
    void parseNavTimeGPSMsg();

    /*!
     * \brief parseSingleBlock - parse input message
     */
    void parseSingleBlock();

    /*!
     * \brief checkFlags - check validity flags
     * \return true if valid, otherwise false
     */
    bool checkFlags();

    /*!
     * \brief setTimeNano - compute time in nanoseconds
     * \brief from input week number, iTow, fTow
     * \return true on success, otherwise false
     */
    bool setTimeNano();
};

#endif // __GNSSNAVTIMEGPSPARSER_H__
