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

#ifndef __GNSSRXMMEASXPARSER_H__
#define __GNSSRXMMEASXPARSER_H__

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
 * Message: UBX-RXM-MEASX
 * Description: Satellite Measurements for RRLP
*/

using ::android::hardware::gnss::V1_0::IGnssMeasurementCallback;
using ::android::hardware::gnss::V1_0::GnssConstellationType;
using ::android::hardware::gnss::V1_0::GnssMax;

typedef IGnssMeasurementCallback::GnssMeasurementState GnssMS;
typedef IGnssMeasurementCallback::GnssMultipathIndicator GnssMI;

class GnssRxmMeasxParser : public GnssParserCommonImpl {
public:
    GnssRxmMeasxParser(const char *payload, uint16_t payloadLen);
    ~GnssRxmMeasxParser(){}

    /*!
     * \brief retrieveSvInfo - fill the gnssData object with collected data
     * \param gnssData - reference to gnssData object, to be filled
     * \return RxmDone on success, otherwise NotReady
     */
    uint8_t retrieveSvInfo(IGnssMeasurementCallback::GnssData &gnssData) final;


    /*!
     * \brief dumpDebug - print log in logcat, and write dump to file
     */
    void dumpDebug() final;


private:
    enum UbxGnssId : uint8_t {
        GPS = 0,
        SBAS = 1,
        GALILEO = 2,
        BEIDOU = 3,
        QZSS = 5,
        GLONASS = 6,
    };

    typedef struct SingleBlock {
        uint8_t version;
        uint8_t numSvs;
        uint32_t gpsTOW;
        uint32_t glonassTOW;
        uint32_t bdsTOW;
        uint32_t qzssTOW;
        uint16_t gpsTOWacc;
        uint16_t glonassTOWacc;
        uint16_t bdsTOWacc;
        uint16_t qzssTOWacc;
        uint8_t TOWset;
    } singleBlock_t;

    typedef struct RepeatedBlock {
        uint8_t gnssId;
        uint8_t svId;
        uint8_t cn0;
        uint8_t multipath;
        int32_t pseudoRangeRate;
    } repeatedBlock_t;


    uint8_t* mPayload = nullptr;
    uint16_t mPayloadLen = 0;

    std::vector<repeatedBlock_t> data;
    singleBlock_t meta;
    /*!
     * \brief mValid - object is valid if all procedures were done successfully
     * \brief denotes if the data could be retrieved
     */
    bool mValid = false;
    GnssMS mTOWstate = GnssMS::STATE_TOW_DECODED;

protected:
    GnssRxmMeasxParser(){}

    /*!
     * \brief parseRxmMeasMsg - collect data from input message and fill meta and data private members
     * \brief parses first block of incoming message, and all repeated blocks
     */
    void parseRxmMeasMsg();

    /*!
     * \brief parseSingleBlock - parse first block of incoming message and fill the meta private member
     * \param msg - a pointer to the beginning of the incoming message
     */
    void parseSingleBlock(const uint8_t* msg);

    /*!
     * \brief parseRepeatedBlock - parse concrete repeated block from the incoming message
     * \param msg - a pointer to the beginning of the repeated block
     */
    void parseRepeatedBlock(const uint8_t* msg);

    /*!
     * \brief parseRepeatedBlocks - parse all repeated blocks from incoming message
     * \param msg - a pointer to the beginning of the first repeated block
     * \param msgLen - length of the repeated blocks in bytes
     */
    void parseRepeatedBlocks(const uint8_t* msg, const uint16_t msgLen);

    /*!
     * \brief getTOWforGnssId - provide corresponding value (TOW) to the concrete constellation from meta private member
     * \brief set mTOWstate GnssMS::STATE_TOW_KNOWN if requested constellation is GALILEO or SBAS
     * \brief set mTOWstate GnssMS::STATE_UNKNOWN if requested constellation unknown
     * \param gnssId - constellation id of UbxGnssId enum
     * \return time of week (TOW) in ms for concrete constellation, otherwise 0  for unknown gnssId
     */
    uint32_t getTOWforGnssId(const uint8_t gnssId);

    /*!
     * \brief getTOWforGnssId - provide corresponding value (TOWacc)to the concrete constellation from meta private member
     * \param gnssId - constellation id of UbxGnssId enum
     * \return TOW accuracy in ms
     */
    uint16_t getTOWaccForGnssId(const uint8_t gnssId);

    /*!
     * \brief setNsFromMs - convert ms to ns
     * \param ms - milliseconds
     * \return nanoseconds
     */
    int64_t setNsFromMs(uint32_t ms);

    /*!
     * \brief getConstellationFromGnssId - convert UbxGnssId to GnssConstellationType
     * \param gnssId - index of constellation in means of UbxGnssId
     * \return constellation in means of GnssConstellationType
     */
    GnssConstellationType getConstellationFromGnssId(const uint8_t gnssId);

    /*!
     * \brief getGnssMeasurement - provide collected data for each Satellite Vehicle
     * \param instance - reference to the hidl_array of GnssMeasurement structures
     * \param block - reference to the repeated block that corresponds to the concrete Satellite Vehicle
     */
    void getGnssMeasurement(IGnssMeasurementCallback::GnssMeasurement &instance, repeatedBlock_t &block);
};

#endif // __GNSSRXMMEASXPARSER_H__
