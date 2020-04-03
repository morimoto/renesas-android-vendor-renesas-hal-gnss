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

#ifndef __GNSSNAVSTATUSPARSER_H__
#define __GNSSNAVSTATUSPARSER_H__

#include <cstdint>

#include <android/hardware/gnss/1.0/types.h>
#include <android/hardware/gnss/1.0/IGnssMeasurementCallback.h>

#include "GnssParserCommonImpl.h"

/* From u-blox 8 / u-blox M8 Receiver Description - Manual
 * 32.18.20 UBX-NAV-STATUS (0x01 0x03)
 * 32.18.20.1 Receiver Navigation Status
 * Message: NAV-STATUS
 * Description: Receiver Navigation Status
*/

class GnssNavStatusParser : public GnssParserCommonImpl {

public:
    /*!
     * \brief GnssNavStatusParser
     * \param payload - a pointer to the payload of incoming message
     * \param payloadLen - length in bytes of payload
     */
    GnssNavStatusParser(const uint8_t* payload, uint16_t payloadLen);
    ~GnssNavStatusParser() override {}

    /*!
     * \brief retrieveSvInfo - fill the gnssData object with collected data
     * \param gnssData - reference to gnssData object
     * \return  StatusDone on success, otherwise NotReady
     */
    uint8_t retrieveSvInfo(MeasurementCb::GnssData &gnssData) final;

    /*!
     * \brief dumpDebug - print log in logcat, and write dump to file
     */
    void dumpDebug() override;

private:
    typedef struct SingleBlock {
        uint32_t iTow;
        uint32_t msss;
    } singleBlock_t;

    int64_t timeNano = 0;
    const uint8_t* mPayload;
    singleBlock_t data = {};
    uint16_t mPayloadLen = 0;

    bool mValid = false;

protected:
    GnssNavStatusParser() : mPayload(nullptr) {}

    /*!
     * \brief parseNavStatusMsg - collect data from input message, set validity
     */
    void parseNavStatusMsg();

    /*!
     * \brief parseSingleBlock - parse input message
     */
    void parseSingleBlock();

    /*!
     * \brief setTimeNano - compute time in nanoseconds
     * \brief from input week number, iTow, fTow
     * \return true on success, otherwise false
     */
    bool setTimeNano();
};

#endif // __GNSSNAVSTATUSPARSER_H__
