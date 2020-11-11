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

#ifndef NMEAGGA_H
#define NMEAGGA_H

#include <NmeaParserCommon.h>

/**
 * @brief GnssLocationFlags
 */
using GnssLocationFlags = android::hardware::gnss::V1_0::GnssLocationFlags;

/**
 * @brief NmeaGga
 *
 * @tparam T
 */
template <typename T>
class NmeaGga : public NmeaParserCommon<T> {
public:
    /**
     * @brief Construct a new Nmea Gga object
     */
    NmeaGga();

    /**
     * @brief Construct a new Nmea Gga object
     *
     * @param in
     * @param inLen
     * @param protocol
     */
    NmeaGga(const char* in, const size_t& inLen,
            const NmeaVersion& protocol);

    /**
     * @brief Construct a new Nmea Gga object
     *
     * @param in
     * @param protocol
     */
    NmeaGga(std::string& in, NmeaVersion& protocol);
    ~NmeaGga() override {}

    NmeaMsgType GetMsgType() override;
    NmeaVersion GetProtocolVersion() override;
    NPError GetData(T out) override;
    bool IsValid() override;

protected:
    /**
     * @brief Parse
     *
     * @return NPError
     */
    NPError Parse();

    /**
     * @brief Parse
     *
     * @param in
     * @return NPError
     */
    NPError Parse(std::string& in) override;

    /**
     * @brief ParseCommon
     *
     * @param gga
     * @return NPError
     */
    NPError ParseCommon(std::vector<std::string>& gga);

    /**
     * @brief Validate Parcel
     *
     * @return NPError
     */
    NPError ValidateParcel();

private:
    typedef struct Parcel {
        double altitude;
        double hdop;
    } parcel_t;

    enum GgaOfst : size_t {
        Hdop     = 8,
        Altitude = 9,
    };

    constexpr static const std::array<size_t, NmeaVersion::AMOUNT>
        mGgaPartsAmount = {15, 15, 15};
    static const NmeaMsgType mType = NmeaMsgType::GGA;

    const char* mPayload;
    const size_t mPayloadLen;

    NmeaVersion mCurrentProtocol = NmeaVersion::NMEAv23;
    bool mIsValid = false;
    parcel_t mParcel;
    uint16_t mFlags = 0u;
};

template <typename T>
NmeaGga<T>::NmeaGga() : mPayload(nullptr),
                        mPayloadLen(0) {
}

template <typename T>
NmeaGga<T>::NmeaGga(std::string& in, NmeaVersion& protocol) : mPayload(in.c_str()),
                                                              mPayloadLen(in.size()),
                                                              mCurrentProtocol(protocol) {
    if (NPError::Success == Parse(in)) {
        mIsValid = true;
    }
}

template <typename T>
NmeaGga<T>::NmeaGga(const char* in, const size_t& inLen,
                    const NmeaVersion& protocol) : mPayload(in),
                                                   mPayloadLen(inLen),
                                                   mCurrentProtocol(protocol) {
    if (NPError::Success == Parse()) {
        mIsValid = true;
    }
}

template <typename T>
NmeaMsgType NmeaGga<T>::GetMsgType() {
    return mType;
}

template <typename T>
NmeaVersion NmeaGga<T>::GetProtocolVersion() {
    return mCurrentProtocol;
}

template <typename T>
NPError NmeaGga<T>::GetData([[maybe_unused]] T out) {
    return NPError::Success;
}

template <typename T>
bool NmeaGga<T>::IsValid() {
    return mIsValid;
}

template <typename T>
NPError NmeaGga<T>::ParseCommon(std::vector<std::string>& gga) {
    if (gga.size() != mGgaPartsAmount[mCurrentProtocol]) {
        return NPError::IncompletePacket;
    }

    if (0 >= gga[GgaOfst::Altitude].length()) {
        return NPError::InvalidData;
    }

    mParcel.altitude = atof(gga[GgaOfst::Altitude].c_str());
    mFlags |= GnssLocationFlags::HAS_ALTITUDE;

    if (0 >= gga[GgaOfst::Hdop].length()) {
        return NPError::InvalidData;
    }

    mParcel.hdop = std::stof(gga[GgaOfst::Hdop]);
    mFlags |= GnssLocationFlags::HAS_HORIZONTAL_ACCURACY;

    return ValidateParcel();
}

template <typename T>
NPError NmeaGga<T>::Parse(std::string& in) {
    if (in.empty()) {
        return NPError::BadInputParameter;
    }

    std::vector<std::string> gga;
    this->Split(in, gga);
    return ParseCommon(gga);
}

template <typename T>
NPError NmeaGga<T>::Parse() {
    if (nullptr == mPayload || mPayloadLen == 0) {
        return NPError::IncompletePacket;
    }

    std::vector<std::string> gga;
    std::string parcel(mPayload, mPayloadLen);
    this->Split(parcel, gga);
    return ParseCommon(gga);
}

//TODO(g.chabukiani): implement validation
template <typename T>
NPError NmeaGga<T>::ValidateParcel() {
    return NPError::Success;
}

#endif  // NMEAGGA_H
