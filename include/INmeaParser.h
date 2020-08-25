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
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <log/log.h>

#include <android/hardware/gnss/2.0/IGnssCallback.h>
#include <android/hardware/gnss/2.0/types.h>

const std::string ggaHeader = "GGA";
const std::string gsaHeader = "GSA";
const std::string gsvHeader = "GSV";
const std::string rmcHeader = "RMC";
const std::string pubx00Header = "PUBX,00";
const std::string txtHeader = "TXT";

const char mNmeaBeginParcel = '$';
const char mNmeaEndParcelCarriageReturn = '\r';
const char mNmeaEndParcelNewLine = '\n';

enum class NPError : uint8_t {
    Success,
    InvalidData,
    UnknownError,
    IncompletePacket,
    BadInputParameter,
    UnknownNmeaVersion,
    UnknownUbxProtocol,
};

enum class NmeaMsgType : uint8_t {
    RMC,
    GGA,
    GSV,
    GSA,
    TXT,
    PUBX00,
};

enum NmeaVersion : size_t {
    NMEAv23 = 0,
    NMEAv40 = 1,
    NMEAv41 = 2,
    AMOUNT = 3,
};

enum NmeaConstellationId : size_t {
    GPS_SBAS_QZSS = 0,
    GLONASS       = 1,
    GALILEO       = 2,
    BEIDOU        = 3,
    ANY           = 4,
    COUNT         = 5,
};

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

typedef ::android::hardware::gnss::V2_0::GnssLocation LocationData;
typedef LocationData& LocationOutType;
typedef std::shared_ptr<INmeaParser<LocationOutType>> LocationQueueType;

typedef struct {
    double altitude;
    float horizontalAcc;
    float verticalAcc;
    uint16_t flags;
} LocationExtraInfo;
typedef LocationExtraInfo& LocationExtraInfoOutType;
typedef std::shared_ptr<INmeaParser<LocationExtraInfoOutType>> LocationExtraInfoQueueType;
typedef std::vector<::android::hardware::gnss::V2_0::IGnssCallback::GnssSvInfo> SvInfoList;
typedef struct {
    NmeaConstellationId gnssId;
    uint8_t msgAmount;
    uint8_t msgNum;
    SvInfoList svInfoList;
} SvInfoType;
typedef SvInfoType& SvInfoOutType;
typedef std::shared_ptr<INmeaParser<SvInfoOutType>> SvInfoQueueType;

typedef struct {
    size_t gnssId;
    std::vector<int64_t> svList;
} SvInfoFixType;
typedef SvInfoFixType& SvInfoFixOutType;
typedef std::shared_ptr<INmeaParser<SvInfoFixOutType>> SvInfoFixQueueType;
