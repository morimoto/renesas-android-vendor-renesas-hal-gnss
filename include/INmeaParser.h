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

#ifndef INMEAPARSER_H
#define INMEAPARSER_H

#include <android/hardware/gnss/2.0/IGnssCallback.h>
#include <android/hardware/gnss/2.1/IGnssCallback.h>

/**
 * @brief ggaHeader
 */
const std::string ggaHeader = "GGA";

/**
 * @brief gsaHeader
 */
const std::string gsaHeader = "GSA";

/**
 * @brief gsvHeader
 */
const std::string gsvHeader = "GSV";

/**
 * @brief rmcHeader
 */
const std::string rmcHeader = "RMC";

/**
 * @brief pubx00Header
 */
const std::string pubx00Header = "PUBX,00";

/**
 * @brief txtHeader
 */
const std::string txtHeader = "TXT";

/**
 * @brief mNmeaBeginParcel
 */
const char mNmeaBeginParcel = '$';

/**
 * @brief mNmeaEndParcelCarriageReturn
 */
const char mNmeaEndParcelCarriageReturn = '\r';

/**
 * @brief mNmeaEndParcelNewLine
 */
const char mNmeaEndParcelNewLine = '\n';

/**
 * @brief NPError
 */
enum class NPError : uint8_t {
    /**
     * @brief Success
     */
    Success,

    /**
     * @brief InvalidData
     */
    InvalidData,

    /**
     * @brief UnknownError
     */
    UnknownError,

    /**
     * @brief IncompletePacket
     */
    IncompletePacket,

    /**
     * @brief BadInputParameter
     */
    BadInputParameter,

    /**
     * @brief UnknownNmeaVersion
     */
    UnknownNmeaVersion,

    /**
     * @brief UnknownUbxProtocol
     */
    UnknownUbxProtocol,
};

/**
 * @brief Nmea Message Type
 */
enum class NmeaMsgType : uint8_t {
    /**
     * @brief RMC
     */
    RMC,

    /**
     * @brief GGA
     */
    GGA,

    /**
     * @brief GSV
     */
    GSV,

    /**
     * @brief GSA
     */
    GSA,

    /**
     * @brief TXT
     */
    TXT,

    /**
     * @brief PUBX00
     */
    PUBX00,
};

/**
 * @brief Nmea protocol version
 */
enum NmeaVersion : size_t {
    /**
     * @brief NMEAv23
     */
    NMEAv23 = 0,

    /**
     * @brief NMEAv40
     */
    NMEAv40 = 1,

    /**
     * @brief NMEAv41
     */
    NMEAv41 = 2,

    /**
     * @brief AMOUNT
     */
    AMOUNT = 3,
};

/**
 * @brief Nmea Constellation Id
 */
enum NmeaConstellationId : size_t {
    /**
     * @brief GPS_SBAS_QZSS
     */
    GPS_SBAS_QZSS = 0,

    /**
     * @brief GLONASS
     */
    GLONASS = 1,

    /**
     * @brief GALILEO
     */
    GALILEO = 2,

    /**
     * @brief BEIDOU
     */
    BEIDOU = 3,

    /**
     * @brief ANY
     */
    ANY = 4,

    /**
     * @brief COUNT
     */
    COUNT = 5,
};

/**
 * @brief Nmea Caller Id Gnss Id Pair
 */
static std::pair<std::string, size_t> NmeaCallerIdGnssIdPair[] = {
    {"GP", 0}, {"GN", 0}, {"GL", 6}, {"GA", 2}, {"GB", 3}};

/**
 * @brief INmeaParser
 *
 * @tparam T
 */
template <typename T>
class INmeaParser {
public:
    INmeaParser() = default;
    virtual ~INmeaParser() = default;

    /*!
     * \brief GetMsgType
     * \return
     */
    virtual NmeaMsgType GetMsgType() = 0;

    /*!
     * \brief GetProtocolVersion
     * \return
     */
    virtual NmeaVersion GetProtocolVersion() = 0;

    /*!
     * \brief GetData
     * \param out
     * \return
     */
    virtual NPError GetData(T out) = 0;

    /*!
     * \brief IsValid
     * \return
     */
    virtual bool IsValid() = 0;
};

/**
 * @brief Location Data
 */
typedef ::android::hardware::gnss::V2_0::GnssLocation LocationData;

/**
 * @brief Location Out Type
 */
typedef LocationData& LocationOutType;

/**
 * @brief Location Queue Type
 */
typedef std::shared_ptr<INmeaParser<LocationOutType>> LocationQueueType;

/**
 * @brief Location Extra Info
 */
typedef struct {
    /**
     * @brief altitude
     */
    double altitude;

    /**
     * @brief horizontalAcc
     */
    float horizontalAcc;

    /**
     * @brief verticalAcc
     */
    float verticalAcc;

    /**
     * @brief flags
     */
    uint16_t flags;
} LocationExtraInfo;

/**
 * @brief Location Extra Info Out Type
 */
typedef LocationExtraInfo& LocationExtraInfoOutType;

/**
 * @brief Location Extra Info Queue Type
 */
typedef std::shared_ptr<INmeaParser<LocationExtraInfoOutType>> LocationExtraInfoQueueType;

/**
 * @brief SvInfo List
 */
typedef std::vector<::android::hardware::gnss::V2_1::IGnssCallback::GnssSvInfo> SvInfoList;

/**
 * @brief SvInfo Type
 */
typedef struct {
    /**
     * @brief gnssId
     */
    NmeaConstellationId gnssId;

    /**
     * @brief msgAmount
     */
    uint8_t msgAmount;

    /**
     * @brief msgNum
     */
    uint8_t msgNum;

    /**
     * @brief svInfoList
     */
    SvInfoList svInfoList;
} SvInfoType;

/**
 * @brief SvInfo Out Type
 */
typedef SvInfoType& SvInfoOutType;

/**
 * @brief SvInfo Queue Type
 */
typedef std::shared_ptr<INmeaParser<SvInfoOutType>> SvInfoQueueType;

/**
 * @brief SvInfo Fix Type
 */
typedef struct {
    /**
     * @brief gnssId
     */
    size_t gnssId;

    /**
     * @brief svList
     */
    std::vector<int64_t> svList;
} SvInfoFixType;

/**
 * @brief SvInfo Fix Out Type
 */
typedef SvInfoFixType& SvInfoFixOutType;

/**
 * @brief SvInfo Fix Queue Type
 */
typedef std::shared_ptr<INmeaParser<SvInfoFixOutType>> SvInfoFixQueueType;

/**
 * @brief talker Id To Gnss Id
 *
 * @param talkerId
 * @return size_t
 */
size_t talkerIdToGnssId(std::string talkerId);

#endif  // INMEAPARSER_H
