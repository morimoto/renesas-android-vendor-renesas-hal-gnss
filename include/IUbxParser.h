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

#ifndef IUBXPARSER_H
#define IUBXPARSER_H

#include <android/hardware/gnss/2.1/IGnss.h>

#include <cstddef>
#include <cstdint>

enum class UPError : uint8_t {
    Success,
    UnknownError,
    UnknownUbxProtocol,
    IncompletePacket,
    InvalidData,
    BadInputParameter,
};

enum class UbxClass : uint8_t {
    NAV = 0x01,
    RXM = 0x02,
    INF = 0x04,
    ACK = 0x05,
    CFG = 0x06,
    MON = 0x0A,
    NMEA_CFG = 0xF0,
    NMEA_CFG_PUBX = 0xF1,
    UNKNOWN = 0xFF,
};

enum class UbxId : uint8_t {
    NACK = 0x00,
    ACK = 0x01,
    STATUS = 0x03,
    RMC = 0x04,
    PVT = 0x07,
    MEASX = 0x14,
    TIMEGPS = 0x20,
    CLOCK = 0x22,
    VER = 0x04,
    NMEA_GLL = 0x01,
    NMEA_VTG = 0x05,
    NMEA_PUBX_POSITION = 0x00,
    CLEAR = 0x09,
    NMEA = 0x17,
    NAV5 = 0x24,
    GNSS = 0x3E,
    UNKNOWN = 0xFF,
};

enum class UbxGnssId : uint8_t {
    GPS = 0,
    SBAS = 1,
    GALILEO = 2,
    BEIDOU = 3,
    QZSS = 5,
    GLONASS = 6,
};

enum class UbxMsg : uint8_t {
    NAV_TIME_UTC,
    NAV_PVT,
    RXM_MEASX,
    NAV_TIME_GPS,
    NAV_TIME_GLONASS,
    NAV_TIME_BEIDU,
    NAV_TIME_GALILEO,
    NAV_STATUS,
    NAV_CLOCK,
    ACK_ACK,
    ACK_NACK,
    MON_VER,
};

enum class UbxProtocolVersion : uint8_t {
    UBX14,
    UBX15,
    UBX16,
    UBX18,
    UBX21,
    UBX23,
};

template <typename T>
class IUbxParser {
public:
    virtual ~IUbxParser() = 0;

    /*!
     * \brief GetMsgType
     * \return
     */
    virtual UbxMsg GetMsgType() = 0;

    /*!
     * \brief GetData
     * \param out
     * \return
     */
    virtual UPError GetData(T out) = 0;

    /*!
     * \brief IsValid
     * \return
     */
    virtual bool IsValid() = 0;
};

template <typename T>
IUbxParser<T>::~IUbxParser() {}

typedef std::pair<UbxClass, UbxId>& AckOutType;
using UbxACK = IUbxParser<AckOutType>;
typedef std::shared_ptr<UbxACK> AckQueueType;
typedef AckQueueType CvAckType;

typedef struct MonVerOut {
    double swVersion;
} monVerOut_t;
typedef monVerOut_t& monVerOut;
typedef std::shared_ptr<IUbxParser<monVerOut>> MonVerQueueType;

#endif // IUBXPARSER_H
