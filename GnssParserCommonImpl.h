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

    /*!
     * \brief isBigEndian - check if current system is big endian
     * \return true if big endian
     */
    bool isBigEndian();

    template <typename T>
    T get(const uint8_t* ptr)
    {
        union
        {
            T out;
            uint8_t in[sizeof(T)];
        } result;

        size_t sizeOfType = sizeof(T);
        for(size_t i = 0; i < sizeOfType; ++i) {
            result.in[i] = ptr[i];
        }

        return result.out;
    }

    template <typename T>
    T convert(const uint8_t* ptr)
    {
        union
        {
            T out;
            uint8_t in[sizeof(T)];
        } result;

        size_t reverseCounter = sizeof(T) - 1;
        for (auto& byte : result.in) {
            byte = ptr[reverseCounter];
            --reverseCounter;
            if (0 == reverseCounter) {
                break;
            }
        }

        return result.out;
    }

public:
    /*!
     * \brief retrieveSvInfo - provide data collected from parsed messages
     * \param gnssData - a reference to fill the object
     * \return status of operation
     */
    virtual uint8_t retrieveSvInfo(MeasurementCb::GnssData &gnssData) override = 0;

    /*!
     * \brief dumpDebug - provide minimal debug log
     */
    virtual void dumpDebug() override;

    virtual ~GnssParserCommonImpl() override {}

protected:
    /*!
     * \brief scaleDown - scaling usin division
     */
    template <typename T1, typename T2>
    T2 scaleDown(T1 val, T2 scale)
    {
        return static_cast<T2>(val) / scale;
    }

    /*!
     * \brief scaleDown - scaling using multiplication
     */
    template <typename T1, typename T2>
    T2 scaleUp(T1 val, T2 scale)
    {
        return static_cast<T2>(val) * scale;
    }

    /*!
     * \brief inRange - check if value is in range
     * \brief if value is not in range, set it to the begin value of the range
     */
    template <typename T1>
    void inRange(T1 begin, T1 end, T1 &value)
    {
        if (value < begin || value > end) {
            value = begin;
        }
    }

    /*!
     * \brief inRanges - check if value is in range
     * \brief if value is not in range, set it to the begin value of the second range
     */
    template <typename T1>
    void inRanges(T1 beginFirst, T1 endFirst, T1 beginSecond, T1 endSecond, T1 &value)
    {
        if (value >= beginFirst && value <= endFirst) {
            return;
        } else if (value >= beginSecond && value <= endSecond) {
            return;
        } else {
            value = beginSecond;
        }
    }

    /*!
     * \brief hexdump - amend write to specified file
     * \param file - a pointer to the file name (absolute path)
     * \param ptr - a pointer to data to be dumped
     * \param buflen - length in bytes of data to be dumped
     */
    void hexdump(const char* file, void* ptr, size_t buflen);

    /*!
     * \brief getValue
     * \param ptr
     * \return
     */
    template <typename T>
    T getValue(const uint8_t* ptr)
    {
        if (nullptr == ptr) {
            return 0;
        }

        if (isBigEndian()) {
            return convert<T>(ptr);
        }

        return get<T>(ptr);
    }
};
#endif // __GNSSPARSERCOMMONIMPL_H__
