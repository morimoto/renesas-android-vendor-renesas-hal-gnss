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

#include "include/IGnssReceiver.h"

enum class Endian : uint8_t {
    Little,
    Big,
    Unset,
};

static constexpr uint32_t defaultBaudRate = 9600;
static constexpr uint32_t badBaudRate = 0;
static constexpr int closedFd = -1;
static constexpr size_t fakeLineMaxSize = 128;

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
     * \brief GetInstance
     * \param receiver
     * \return
     */
    static Transport& GetInstance(const
            std::shared_ptr<IGnssReceiver>& receiver) {
        static Transport instance(receiver);
        return instance;
    }

    /*!
     * \brief Transport - destructor
     */
    ~Transport();

    /*!
     * \brief ResetTransport
     * \param receiver
     * \return
     */
    TError ResetTransport(const std::shared_ptr<IGnssReceiver>& receiver);

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
    TError GetTransportState();

    /*!
     * \brief Transport::GetEndianType
     * \return
     */
    Endian GetEndianType() const;

    /*!
     * \brief Transport::ResetDevice
     * \return
     */
    TError ResetGnssReceiver();

protected:
    /*!
     * \brief Open
     * \return
     */
    TError Open(GnssReceiverType recType);

    /*!
     * \brief OpenFakeFile
     * \return
     */
    TError OpenFakeFile();

    /*!
     * \brief OpenTTY
     * \return
     */
    TError OpenTTY();

    /*!
    * \brief ReadByte
    * \param errCode
    * \return
    */
    //    uint8_t ReadByte(TError& errCode);
    char ReadByte(TError& errCode);

    /*!
     * \brief Close
     */
    TError Close();

    /*!
     * \brief SetUpLine
     * \return
     */
    TError SetUpLine();

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

private:
    Transport() = delete;
    Transport(const std::shared_ptr<IGnssReceiver>& receiver);
    Transport(Transport const&) = delete;
    Transport& operator=(Transport const&) = delete;
    std::string mPath;
    uint32_t mBaudRate = badBaudRate;
    TError mState = TError::TransportNotReady;
    int mFd = closedFd;
    std::fstream fakeStream;
    std::mutex readWriteLock;
    Endian mEndianType = Endian::Unset;
    GnssReceiverType mReceiverType;
    const uint8_t mSync1 = 0xB5;
    const uint8_t mSync2 = 0x62;
};

template <typename T>
TError Transport::Read(std::vector<T>& out) {
    std::lock_guard<std::mutex> lock(readWriteLock);
    TError result = TError::FailedToRead;

    if constexpr (std::is_same_v<char, T>) {
        out.push_back(ReadByte(result));
    } else if constexpr (std::is_same_v<std::string, T>) {
        std::array<char, fakeLineMaxSize> line;
        fakeStream.getline(line.data(), line.size());

        if (!fakeStream.fail()) {
            result = TError::Success;
            out.push_back({line.data()});
        }
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
    std::lock_guard<std::mutex> lock(readWriteLock);
    std::vector<uint8_t> toWrite;
    toWrite.push_back(mSync1);
    toWrite.push_back(mSync2);

    for (const auto& element : in) {
        toWrite.push_back(element);
    }

    AddCheckSum(in, toWrite);

    if (closedFd < mFd) {
        ALOGV("%s, going to write", __func__);
        ssize_t ret = ::write(mFd, toWrite.data(), toWrite.size());

        if (ret == static_cast<ssize_t>(toWrite.size())) {
            ALOGV("%s, success", __func__);
            return TError::Success;
        }
    }

    ALOGE("%s, failed_to_write", __func__);
    return TError::FailedToWrite;
}
