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

#include "include/IUbxParser.h"

enum class Endianess : uint8_t {
    Little,
    Big,
    Unset,
};

static Endianess native = Endianess::Unset;

template <typename ClassType>
class UbxParserCommon : public IUbxParser<ClassType> {
public:
    UbxParserCommon();
    virtual ~UbxParserCommon() {}
    static const int64_t msToNsMultiplier = 1000000;

protected:
    /*!
     * \brief ScaleDown - scaling usin division
     */
    template <typename T1, typename T2>
    T2 ScaleDown(T1 val, T2 scale);

    /*!
     * \brief ScaleDown - scaling using multiplication
     */
    template <typename T1, typename T2>
    T2 ScaleUp(T1 val, T2 scale);

    /*!
     * \brief inRange - check if value is in range
     * \brief if value is not in range, set it to the begin value of the range
     */
    template <typename T1>
    void InRange(T1 begin, T1 end, T1& value);

    /*!
     * \brief inRanges - check if value is in range
     * \brief if value is not in range,
     *        set it to the begin value of the second range
     */
    template <typename T1>
    void InRanges(T1 beginFirst, T1 endFirst, T1 beginSecond, T1 endSecond,
                  T1& value);

    /*!
     * \brief GetValue - get value of requested type from byte array
     * \param ptr - a pointer to input array with needed value
     * \return
     */
    template <typename T>
    T GetValue(const uint8_t* ptr);

    /*!
     * \brief Get - get value without endian conversion
     * \param ptr - a pointer to input array with needed value
     * \return
     */
    template <typename T>
    T Get(const uint8_t* ptr);

    /*!
     * \brief Convert - get value with convertion to big endian
     * \param ptr - a pointer to input array with needed value
     * \return
     */
    template <typename T>
    T Convert(const uint8_t* ptr);

    /*!
     * \brief isBigEndian - check if current system is big endian
     * \return true if big endian
     */
    bool IsBigEndian();

    /*!
     * \brief GetEndian - detect endianness
     * \return true if big endian, otherwise false
     */
    bool GetEndian(Endianess& param);
};

template <typename ClassType>
bool UbxParserCommon<ClassType>::IsBigEndian() {
    switch (native) {
    case Endianess::Big:
        return true;

    case Endianess::Little:
        return false;

    default:
        return GetEndian(native);
    }
}

//TODO(g.chabukiani): refactor this place
//as Transport unit should get endian type!
template <typename ClassType>
bool UbxParserCommon<ClassType>::GetEndian(Endianess& out) {
    bool result = false;
    union {
        uint32_t dec;
        char literal[4];
    } checker = {0x01020304};

    if (0x01 == checker.literal[0]) {
        out = Endianess::Big;
        result = true;
    } else {
        out = Endianess::Little;
    }

    return result;
}

template <typename ClassType>
template <typename T1, typename T2>
T2 UbxParserCommon<ClassType>::ScaleDown(T1 val, T2 scale) {
    return static_cast<T2>(val) / scale;
}

template <typename ClassType>
template <typename T1, typename T2>
T2 UbxParserCommon<ClassType>::ScaleUp(T1 val, T2 scale) {
    return static_cast<T2>(val) * scale;
}

template <typename ClassType>
template <typename T1>
void UbxParserCommon<ClassType>::InRange(T1 begin, T1 end, T1& value) {
    if (value < begin || value > end) {
        value = begin;
    }
}

template <typename ClassType>
template <typename T1>
void UbxParserCommon<ClassType>::InRanges(T1 beginFirst, T1 endFirst,
        T1 beginSecond, T1 endSecond,
        T1& value) {
    if (value >= beginFirst && value <= endFirst) {
        return;
    } else if (value >= beginSecond && value <= endSecond) {
        return;
    } else {
        value = beginSecond;
    }
}

template <typename ClassType>
template <typename T>
T UbxParserCommon<ClassType>::GetValue(const uint8_t* ptr) {
    if (nullptr == ptr) {
        return 0;
    }

    if (IsBigEndian()) {
        return Convert<T>(ptr);
    }

    return Get<T>(ptr);
}

template <typename ClassType>
template <typename T>
T UbxParserCommon<ClassType>::Get(const uint8_t* ptr) {
    union {
        T out;
        uint8_t in[sizeof(T)];
    } result;
    size_t typeSize = sizeof(T);

    for (size_t i = 0; i < typeSize; ++i) {
        result.in[i] = ptr[i];
    }

    return result.out;
}

template <typename ClassType>
template <typename T>
T UbxParserCommon<ClassType>::Convert(const uint8_t* ptr) {
    union {
        T out;
        uint8_t in[sizeof(T)];
    } result;
    size_t reverseCounter = sizeof(T) - 1;

    for (auto& byte : result.in) {
        byte = ptr[reverseCounter];
        --reverseCounter;
    }

    return result.out;
}

template <typename ClassType>
UbxParserCommon<ClassType>::UbxParserCommon() {}
