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

#include <memory>
#include <thread>
#include <atomic>

#include "include/IReader.h"

class FakeReader : public IReader {
public:
    FakeReader() = default;

    /*!
     * \brief FakeReader
     * \param transport
     */
    FakeReader(Transport& transport);

    ~FakeReader() override;
    /*!
     * \brief Start
     * \return
     */
    RDError Start() override;

    /*!
     * \brief Stop
     * \return
     */
    RDError Stop() override;

    /*!
     * \brief SetUpNotificationCallback
     * \param notificationCb
     * \return
     */
    RDError SetUpNotificationCallback(cbPtr notificationCb) override;

protected:
    /*!
     * \brief LoadFakeTxt
     * \return
     */
    RDError LoadFakeTxt();

    /*!
     * \brief SplitLine
     * \param inLine
     * \param out
     * \return
     */
    RDError SplitLine(std::string inLine, std::vector<std::string>& out);

    /*!
     * \brief CalculateDistance
     * \return
     */
    RDError CalculateDistance();

    /*!
     * \brief CalculateBearing
     * \return
     */
    RDError CalculateBearing();

    /*!
     * \brief ReadingLoop
     */
    void ReadingLoop();

private:
    enum FakeLocationPartsOffset : size_t {
        latitudeOffset = 0,
        longitudeOffset = 1,
        speedOffset = 2,
    };

    static const std::string mFakeRoute;
    const size_t mPartsPerLine = 3;
    std::vector<fakeLocationPoint_t> mFakeLocationPoints;
    std::vector<std::string> mRawLocationInput;
    cbPtr mDeathNotificationCallback = nullptr;
    std::fstream mFile;
    std::thread mReadingLoop;
    std::atomic<bool> mExitThread;
    Transport& mTransport;
    std::mutex mLock;
};
