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
#pragma once

#include <cstring>

#include "include/NmeaParserCommon.h"


template <typename T>
class NmeaRmc : public NmeaParserCommon<T> {
public:
    NmeaRmc();
    NmeaRmc(const char* in, const size_t& inLen,
            const NmeaVersion& protocol);
    NmeaRmc(std::string& in, const NmeaVersion& protocol);
    ~NmeaRmc() override {}

    NmeaMsgType GetMsgType() override;
    NmeaVersion GetProtocolVersion() override;
    NPError GetData(T out) override;
    bool IsValid() override;

protected:
    NPError Parse();
    NPError Parse(std::string& in);
    NPError ParseCommon(std::vector<std::string>& rmc);
    NPError ValidateParcel();
    NPError SetTime(const std::string& date, const std::string& time);
    NPError SetLocation(const std::string& lat, const std::string& lon,
                const std::string& northSouth, const std::string& eastWest);
    NPError SetMotion(const std::string& speed, const std::string& cog);

private:
    typedef struct Parcel {
        double lat;
        double lon;
        double alt;
        float speed;
        float cog;
        int64_t time;
        uint8_t status;
    } parcel_t;

    enum RmcOfst : size_t {
        time = 1,
        status = 2,
        latitude = 3,
        northSouthId = 4,
        logntitude = 5,
        eastWestId = 6,
        speed = 7,
        cog = 8,
        date = 9,
    };

    constexpr static const std::array<size_t, NmeaVersion::AMOUNT>
                mRmcPartsAmount = {13, 14, 14};
    constexpr static const float knotToKmph = 1.852f; //knots to km per hour
    constexpr static const float mpsToKmph = 3.6f; //km/h to m/h
    constexpr static const unsigned int secsPerHour = 3600; // seconds per hour
    static const NmeaMsgType mType = NmeaMsgType::RMC;
    static const int minutesPerDegree = 60;
    static const std::string statusActive;
    static const std::string mWest;
    static const std::string mSouth;
    static constexpr float speedAccUblox7 =
        0.1f; // m/s according to NEO-7 datasheet
    static constexpr float bearingAccUblox7 =
        0.5f; // degrees according to NEO-7 datasheet
    static constexpr float speedAccUblox8 =
        0.05f; // m/s according to NEO-8 datasheet
    static constexpr float bearingAccUblox8 =
        0.3f; // degrees according to NEO-8 datasheet

    const char* mPayload;
    const size_t mPayloadLen;

    bool mIsValid = false;
    NmeaVersion mCurrentProtocol = NmeaVersion::NMEAv23;
    parcel_t mParcel;
    uint16_t mFlags;
    float mSpeedAccuracy;
    float mBearingAccuracy;

    void SetConstants();
};

template <typename T>
const std::string NmeaRmc<T>::statusActive = "A";

template <typename T>
const std::string NmeaRmc<T>::mWest = "W";

template <typename T>
const std::string NmeaRmc<T>::mSouth = "S";

template <typename T>
NmeaRmc<T>::NmeaRmc() :
    mPayload(nullptr),
    mPayloadLen(0) {
}

template <typename T>
NmeaRmc<T>::NmeaRmc(std::string& in, const NmeaVersion& protocol) :
    mPayload(in.c_str()),
    mPayloadLen(in.size()),
    mCurrentProtocol(protocol) {
    if (NPError::Success == Parse(in)) {
        SetConstants();
        mIsValid = true;
    }
}

template <typename T>
NmeaRmc<T>::NmeaRmc(const char* in, const size_t& inLen,
                    const NmeaVersion& protocol) :
    mPayload(in),
    mPayloadLen(inLen),
    mCurrentProtocol(protocol) {
    if (NPError::Success == Parse()) {
        SetConstants();
        mIsValid = true;
    }
}

template <typename T>
void NmeaRmc<T>::SetConstants() {
    switch (mCurrentProtocol) {
    case NmeaVersion::NMEAv23:
        mSpeedAccuracy = speedAccUblox7;
        mBearingAccuracy = bearingAccUblox7;
        break;

    case NmeaVersion::NMEAv41:
        mSpeedAccuracy = speedAccUblox8;
        mBearingAccuracy = bearingAccUblox8;
        break;

    default:
        mSpeedAccuracy = 0.0;
        mBearingAccuracy = 0.0;
        break;
    }
}

template <typename T>
NmeaMsgType NmeaRmc<T>::GetMsgType() {
    return mType;
}

template <typename T>
NmeaVersion NmeaRmc<T>::GetProtocolVersion() {
    return mCurrentProtocol;
}

//TODO(g.chabukiani): implement get data
template <typename T>
NPError NmeaRmc<T>::GetData(T out) {
    (void)out;
    return NPError::Success;
}

template <typename T>
bool NmeaRmc<T>::IsValid() {
    return mIsValid;
}

template <typename T>
NPError NmeaRmc<T>::SetTime(const std::string& date, const std::string& time) {
    if (date.size() == 0 || time.size() == 0) {
        return NPError::BadInputParameter;
    }

    tm t;

    // Calc real UTC offset from localtime
    time_t lt = std::time(nullptr);
    localtime_r(&lt, &t);
    long int utc_offset = t.tm_gmtoff - t.tm_isdst * secsPerHour;

    memset(&t, 0, sizeof(struct tm));
    // UTC fix time
    sscanf(time.c_str(), "%02u%02u%02u", &t.tm_hour, &t.tm_min, &t.tm_sec);
    // Date
    sscanf(date.c_str(), "%02u%02u%02u", &t.tm_mday, &t.tm_mon, &t.tm_year);
    // fix month (1-12 -> 0-11)
    t.tm_mon -= 1;
    // fix year (1917 -> 2017)
    t.tm_year += 100;
    // timestamp of the event in milliseconds, mktime(&t) returns seconds,
    // therefore we need to convert
    mParcel.time = (mktime(&t) + utc_offset) * 1000;
    return NPError::Success;
}

//TODO(g.chabukiani): remove magic constants 100
template <typename T>
NPError NmeaRmc<T>::SetLocation(const std::string& lat,
                                const std::string& lon,
                                const std::string& northSouth,
                                const std::string& eastWest) {
    if (lat.size() == 0 || lon.size() == 0) {
        return NPError::BadInputParameter;
    }

    double raw = atof(lat.c_str());
    double polarity = static_cast<double>(northSouth == mSouth ? -1 : 1);
    int degree = static_cast<int>(raw / 100);
    double minutes = ((raw / 100) - degree) * 100;
    mParcel.lat = ((minutes / minutesPerDegree) + degree) * polarity;
    raw = atof(lon.c_str());
    polarity = static_cast<double>(eastWest == mWest ? -1.f : 1.f);
    degree = static_cast<int>(raw / 100);
    minutes = ((raw / 100) - degree) * 100;
    mParcel.lon = ((minutes / minutesPerDegree) + degree) * polarity;
    return NPError::Success;
}

template <typename T>
NPError NmeaRmc<T>::SetMotion(const std::string& speed,
                              const std::string& cog) {
    if (speed.size() == 0 || cog.size() == 0) {
        return NPError::BadInputParameter;
    }

    mParcel.speed = (static_cast<float>(atof(speed.c_str())) * knotToKmph) /
                    mpsToKmph;
    mParcel.cog = static_cast<float>(atof(cog.c_str()));
    return NPError::Success;
}

template <typename T>
NPError NmeaRmc<T>::ParseCommon(std::vector<std::string>& rmc) {
    if (rmc.size() != mRmcPartsAmount[mCurrentProtocol]) {
        return NPError::IncompletePacket;
    }

    if (rmc[RmcOfst::status] != statusActive) {
        return NPError::InvalidData;
    }

    auto result = SetTime(rmc[RmcOfst::date], rmc[RmcOfst::time]);

    if (NPError::Success != result) {
        return result;
    }

    result = SetLocation(rmc[RmcOfst::latitude], rmc[RmcOfst::logntitude],
                         rmc[RmcOfst::northSouthId], rmc[RmcOfst::eastWestId]);

    if (NPError::Success != result) {
        return result;
    }

    result = SetMotion(rmc[RmcOfst::speed], rmc[RmcOfst::cog]);

    if (NPError::Success != result) {
        return result;
    }

    return ValidateParcel();
}

template <typename T>
NPError NmeaRmc<T>::Parse(std::string& in) {
    if (in.empty()) {
        return NPError::IncompletePacket;
    }

    std::vector<std::string> rmc;
    this->Split(in, rmc);
    return ParseCommon(rmc);
}

template <typename T>
NPError NmeaRmc<T>::Parse() {
    if (nullptr == mPayload || mPayloadLen == 0) {
        return NPError::IncompletePacket;
    }

    std::vector<std::string> rmc;
    std::string parcel(mPayload, mPayloadLen);
    this->Split(parcel, rmc);
    return ParseCommon(rmc);
}

//TODO(g.chabukiani): implement validation according to test requirements
//TODO(ivan.o.korniienko): set flags according to the data we have
template <typename T>
NPError NmeaRmc<T>::ValidateParcel() {
    return NPError::Success;
}
