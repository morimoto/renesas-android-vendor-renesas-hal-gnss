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

#ifndef MEASUREMENTBUILDER_H
#define MEASUREMENTBUILDER_H

#include <android/hardware/gnss/2.1/IGnssMeasurement.h>

#include <IUbxParser.h>
#include <MessageQueue.h>

namespace android::hardware::gnss::V2_1::renesas {

/**
 * @brief Error return type
 */
enum MBError : int8_t {
    /**
     * @brief Message with measurements was built successfully
     */
    SUCCESS = 0,

    /**
     * @brief some messages are missing
     */
    INCOMPLETE = -100,

    /**
     * @brief some messages are invalid
     */
    INVALID = -101,

    /**
     * @brief no parsers in MessageQueue
     */
    EMPTY = -102
};

/**
 * @brief Gnss Data
 */
typedef android::hardware::gnss::V2_1::IGnssMeasurementCallback::GnssData GnssData;

/**
 * @brief MBType
 */
typedef std::shared_ptr<IUbxParser<GnssData*>> MBType;

/**
 * @brief MBParserMap
 */
typedef std::map<UbxMsg, MBType> MBParserMap;

/**
 * @brief Measurement Builder implementation
 */
class MeasurementBuilder {
public:
    /*!
     * \brief      Constructor
     */
    MeasurementBuilder();

    /*!
     * \brief      Destructor
     */
    ~MeasurementBuilder() {}

    /*!
     * \brief      Builds message with GNSS measurement data
     *             of messages collected from receivers
     *
     * \param      outData  The structure to be filled with measurement data
     *
     * \return     SUCCESS - message with measurements was built successfully,
     *             INCOMPLETE - some messages from receivers are missing,
     *             INVALID - received GNSS data is invalid,
     *             EMPTY - MessageQueue of MBType is empty
     */
    MBError Build(GnssData& outData);

protected:
    /*!
     * \brief      Selects Ublox parsers needed to fill GnssData structure from
     *             MessageQueue
     *
     * \param      outMap  The map to be filled with expexted parsers. Function
     *                     guarantees that all parsers will have have different
     *                     type, but doesn't check whether they're valid
     *
     * \return     SUCCESS - upon success,
     *             INCOMPLETE - queue doesn't contain some type(s) of messages,
     *             EMPTY - MessageQueue is empty
     */
    MBError GetParsers(MBParserMap& outMap);

private:
    MeasurementBuilder(MeasurementBuilder&) = delete;
    MeasurementBuilder& operator=(const MeasurementBuilder&) = delete;
    void AddElapsedRealtime(GnssData& outData);
    MessageQueue& mMsgQueue;
    std::condition_variable& mCondVar;
    std::mutex mLock;
};

} //namespace android::hardware::gnss::V2_1::renesas

#endif // MEASUREMENTBUILDER_H
