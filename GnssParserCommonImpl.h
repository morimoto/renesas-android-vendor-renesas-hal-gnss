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

#ifndef __GNSSPARSERCOMMONIMPL_H__
#define __GNSSPARSERCOMMONIMPL_H__

#include "GnssIParser.h"

class GnssParserCommonImpl : public GnssIParser {
public:
    /*!
     * \brief retrieveSvInfo - provide data collected from parsed messages
     * \param gnssData - a reference to fill the object
     * \return status of operation
     */
    virtual uint8_t retrieveSvInfo(IGnssMeasurementCallback::GnssData &gnssData) = 0;

    /*!
     * \brief dumpDebug - provide minimal debug log
     */
    virtual void dumpDebug();

    virtual ~GnssParserCommonImpl() {}

protected:
    /*!
     * \brief getUint16 - get two byte integer represented in little endian
     * \param ptr - a pointer to a byte array
     * \return integer in little endian
     */
    uint16_t getUint16(const uint8_t* ptr);

    /*!
     * \brief getUint32 - get four byte integer represented in little endian
     * \param ptr - a pointer to a byte array
     * \return integer in little endian
     */
    uint32_t getUint32(const uint8_t* ptr);

    /*!
     * \brief scaleDown - scaling usin division
     */
    template <typename T1, typename T2>
    T2 scaleDown(T1 val, T2 scale)
    {
        return static_cast<T2>(val/scale);
    }

    /*!
     * \brief scaleDown - scaling using multiplication
     */
    template <typename T1, typename T2>
    T2 scaleUp(T1 val, T2 scale)
    {
        return static_cast<T2>(val*scale);
    }

    /*!
     * \brief hexdump - amend write to specified file
     * \param file - a pointer to the file name (absolute path)
     * \param ptr - a pointer to data to be dumped
     * \param buflen - length in bytes of data to be dumped
     */
    void hexdump(const char *file, void* ptr, size_t buflen);
};
#endif // __GNSSPARSERCOMMONIMPL_H__
