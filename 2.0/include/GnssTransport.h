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

#include <type_traits>
#include <log/log.h>
#include <fstream>
#include <mutex>
#include <array>
#include <vector>

namespace android::hardware::gnss::V2_0::renesas {

enum class Endian : uint8_t {
    Little,
    Big,
    Unset,
};

enum class TError : uint8_t {
    Success,
    FailedToRead,
    FailedToWrite,
    TransportReady,
    TransportNotReady,
    InternalError,
};

class Transport {
public:

    /*!
     * \brief Transport - destructor
     */
    virtual ~Transport();

    /*!
     * \brief Reset
     * \param receiver
     * \return
     */
    virtual TError Reset();

    /*!
     * \brief Read
     * \param out
     * \return
     */
    template <typename T>
    TError Read(std::vector<T>& out);

    /*!
     * \brief Write
     * \param in
     * \return
     */
    template <std::size_t size>
    TError Write(const std::array<uint8_t, size>& in);

    /*!
     * \brief GetTransportState
     * \return
     */
    virtual TError GetTransportState();

    /*!
     * \brief Transport::GetEndianType
     * \return
     */
    Endian GetEndianType() const;

    /*!
     * \brief Get TTY path for transport
     * \return path string
     */
    std::string GetPath() const;
protected:
    /*!
     * \brief Open
     * \return
     */
    virtual TError Open() = 0;

    /*!
    * \brief ReadByte
    * \param errCode
    * \return
    */
    virtual char ReadByte(TError& errCode) = 0;

    /*!
    * \brief Write data to transport
    * \param toWrite vector of data to write
    * \return
    */
    virtual TError WriteData(const std::vector<uint8_t> &toWrite) = 0;

    /*!
     * \brief Close
     */
    virtual TError Close() = 0;

    /*!
     * \brief SetUp
     * \return
     */
    virtual TError SetUp();

    /*!
     * \brief SetEndianness
     * \return
     */
    TError SetEndianness();

    /*!
     * \brief AddCheckSum
     * \param payload
     * \param out
     * \return
     */
    template <std::size_t size>
    TError AddCheckSum(const std::array<uint8_t, size>& payload,
                       std::vector<uint8_t>& out);
    Transport(const std::string &filePath);

    std::mutex readLock, writeLock;
private:
    Transport(Transport const&) = delete;
    Transport& operator=(Transport const&) = delete;

    std::string mFilePath;
    TError mState = TError::TransportNotReady;
    Endian mEndianType = Endian::Unset;

    constexpr static uint8_t mSync1 = 0xB5;
    constexpr static uint8_t mSync2 = 0x62;
};

template <typename T>
TError Transport::Read(std::vector<T>& out) {
    std::lock_guard<std::mutex> lock(readLock);
    TError result = TError::FailedToRead;

    if constexpr (std::is_same_v<char, T>) {
        out.push_back(ReadByte(result));
    }

    return result;
}

template <std::size_t size>
TError Transport::AddCheckSum(const std::array<uint8_t, size>& payload,
                              std::vector<uint8_t>& out) {
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;

    for (const auto& element : payload) {
        ck_a += element;
        ck_b += ck_a;
    }

    out.push_back(ck_a);
    out.push_back(ck_b);
    return TError::Success;
}

template <std::size_t size>
TError Transport::Write(const std::array<uint8_t, size>& in) {
    std::vector<uint8_t> toWrite;
    toWrite.push_back(mSync1);
    toWrite.push_back(mSync2);

    for (const auto& element : in) {
        toWrite.push_back(element);
    }

    AddCheckSum(in, toWrite);
    std::lock_guard<std::mutex> lock(writeLock);
    return WriteData(toWrite);
}

} // namespace android::hardware::gnss::V2_0::renesas
