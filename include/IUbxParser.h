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

/**
 * @brief return error type
 */
enum class UPError : uint8_t {
    /**
     * @brief Success
     */
    Success,

    /**
     * @brief Unknown Error
     */
    UnknownError,

    /**
     * @brief Unknown Ubx Protocol
     */
    UnknownUbxProtocol,

    /**
     * @brief Incomplete Packet
     */
    IncompletePacket,

    /**
     * @brief Invalid Data
     */
    InvalidData,

    /**
     * @brief Bad Input Parameter
     */
    BadInputParameter,
};

/**
 * @brief Ubx message class
 */
enum class UbxClass : uint8_t {
    /**
     * @brief NAV
     */
    NAV = 0x01,

    /**
     * @brief RXM
     */
    RXM = 0x02,

    /**
     * @brief INF
     */
    INF = 0x04,

    /**
     * @brief ACK
     */
    ACK = 0x05,

    /**
     * @brief CFG
     */
    CFG = 0x06,

    /**
     * @brief MON
     */
    MON = 0x0A,

    /**
     * @brief NMEA_CFG
     */
    NMEA_CFG = 0xF0,

    /**
     * @brief NMEA_CFG_PUBX
     */
    NMEA_CFG_PUBX = 0xF1,

    /**
     * @brief UNKNOWN
     */
    UNKNOWN = 0xFF,
};

/**
 * @brief Ubx message Id
 */
enum class UbxId : uint8_t {
    /**
     * @brief NACK
     */
    NACK = 0x00,

    /**
     * @brief ACK
     */
    ACK = 0x01,

    /**
     * @brief STATUS
     */
    STATUS = 0x03,

    /**
     * @brief RMC
     */
    RMC = 0x04,

    /**
     * @brief PVT
     */
    PVT = 0x07,

    /**
     * @brief MEASX
     */
    MEASX = 0x14,

    /**
     * @brief TIMEGPS
     */
    TIMEGPS = 0x20,

    /**
     * @brief CLOCK
     */
    CLOCK = 0x22,

    /**
     * @brief VER
     */
    VER = 0x04,

    /**
     * @brief NMEA_GLL
     */
    NMEA_GLL = 0x01,

    /**
     * @brief NMEA_VTG
     */
    NMEA_VTG = 0x05,

    /**
     * @brief NMEA_PUBX_POSITION
     */
    NMEA_PUBX_POSITION = 0x00,

    /**
     * @brief CLEAR
     */
    CLEAR = 0x09,

    /**
     * @brief NMEA
     */
    NMEA = 0x17,

    /**
     * @brief NAV5
     */
    NAV5 = 0x24,

    /**
     * @brief GNSS
     */
    GNSS = 0x3E,

    /**
     * @brief UNKNOWN message Id
     */
    UNKNOWN = 0xFF,
};

/**
 * @brief Ubx Gnss Id
 */
enum class UbxGnssId : uint8_t {
    /**
     * @brief GPS
     */
    GPS = 0,

    /**
     * @brief SBAS
     */
    SBAS = 1,

    /**
     * @brief GALILEO
     */
    GALILEO = 2,

    /**
     * @brief BEIDOU
     */
    BEIDOU = 3,

    /**
     * @brief QZSS
     */
    QZSS = 5,

    /**
     * @brief GLONASS
     */
    GLONASS = 6,
};

/**
 * @brief Ubx Msg
 */
enum class UbxMsg : uint8_t {
    /**
     * @brief NAV_TIME_UTC
     */
    NAV_TIME_UTC,

    /**
     * @brief NAV_PVT
     */
    NAV_PVT,

    /**
     * @brief RXM_MEASX
     */
    RXM_MEASX,

    /**
     * @brief NAV_TIME_GPS
     */
    NAV_TIME_GPS,

    /**
     * @brief NAV_TIME_GLONASS
     */
    NAV_TIME_GLONASS,

    /**
     * @brief NAV_TIME_BEIDU
     */
    NAV_TIME_BEIDU,

    /**
     * @brief NAV_TIME_GALILEO
     */
    NAV_TIME_GALILEO,

    /**
     * @brief NAV_STATUS
     */
    NAV_STATUS,

    /**
     * @brief NAV_CLOCK
     */
    NAV_CLOCK,

    /**
     * @brief ACK_ACK
     */
    ACK_ACK,

    /**
     * @brief ACK_NACK
     */
    ACK_NACK,

    /**
     * @brief MON_VER
     */
    MON_VER,
};

/**
 * @brief Ubx Protocol Version
 *
 */
enum class UbxProtocolVersion : uint8_t {
    /**
     * @brief UBX14
     */
    UBX14,

    /**
     * @brief UBX15
     */
    UBX15,

    /**
     * @brief UBX16
     */
    UBX16,

    /**
     * @brief UBX18
     */
    UBX18,

    /**
     * @brief UBX21
     */
    UBX21,

    /**
     * @brief UBX23
     */
    UBX23,
};

/**
 * @brief IUbxParser
 *
 * @tparam T
 */
template <typename T>
class IUbxParser {
public:
    /**
     * @brief Destroy the IUbxParser object
     *
     */
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

/**
 * @brief AckOutType
 */
typedef std::pair<UbxClass, UbxId>& AckOutType;

/**
 * @brief UbxACK
 */
using UbxACK = IUbxParser<AckOutType>;

/**
 * @brief AckQueueType
 */
typedef std::shared_ptr<UbxACK> AckQueueType;

/**
 * @brief CvAckType
 */
typedef AckQueueType CvAckType;

/**
 * @brief MonVerOut
 */
typedef struct MonVerOut {
    /**
     * @brief swVersion
     */
    double swVersion;
} monVerOut_t;

/**
 * @brief monVerOut
 */
typedef monVerOut_t& monVerOut;

/**
 * @brief MonVerQueueType
 */
typedef std::shared_ptr<IUbxParser<monVerOut>> MonVerQueueType;

#endif  // IUBXPARSER_H
