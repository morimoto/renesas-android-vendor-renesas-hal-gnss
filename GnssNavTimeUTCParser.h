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

#ifndef __GNSSNAVTIMEUTCPARSER_H__
#define __GNSSNAVTIMEUTCPARSER_H__

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
 * Message: UBX-NAV-TIMEUTC
 * Description: Satellite Measurements for RRLP
*/

using ::android::hardware::gnss::V1_0::IGnssMeasurementCallback;
using ::android::hardware::gnss::V1_0::GnssConstellationType;

class GnssNavTimeUTCParser : public GnssParserCommonImpl {

public:
    /*!
     * \brief GnssNavTimeUTCParser
     * \param payload - a pointer to the payload of incoming message
     * \param payloadLen - length in bytes of payload
     */
    GnssNavTimeUTCParser(const char* payload, uint16_t payloadLen);
    ~GnssNavTimeUTCParser(){}

    /*!
     * \brief retrieveSvInfo - fill the gnssData object with collected data
     * \param gnssData - reference to gnssData object
     * \return  UTCDone on success, otherwise NotReady
     */
    uint8_t retrieveSvInfo(IGnssMeasurementCallback::GnssData &gnssData) final;

    /*!
     * \brief dumpDebug - print log in logcat, and write dump to file
     */
    void dumpDebug() override;

private:
    typedef struct SingleBlock {
        uint32_t iTow;
        uint32_t timeAccuracy;
        int32_t nanoSecondFraction;
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint8_t flags;

    } singleBlock_t;

    uint8_t* mPayload = nullptr;
    uint16_t mPayloadLen = 0;

    singleBlock_t data;
    int64_t timeNano = 0;
    bool mValid = false;

protected:
    GnssNavTimeUTCParser(){}

    /*!
     * \brief parseNavTimeUTCMsg - collect data from input message, set validity
     */
    void parseNavTimeUTCMsg();

    /*!
     * \brief parseSingleBlock - parse input message
     */
    void parseSingleBlock();

    /*!
     * \brief checkFlags - check validity flags
     * \brief check valid iTOW, week number, UTC time TC standard in use
     * \brief check UTC standard in use
     * \return true if valid Week and UTC time, otherwise false
     */
    bool checkFlags();

    /*!
     * \brief setTimeNano - compute time in nanoseconds
     * \brief from input year/month/day/hour/minute/second/nanosecond fraction
     * \return true on success, otherwise false
     */
    bool setTimeNano();
};

#endif // __GNSSNAVTIMEUTCPARSER_H__
