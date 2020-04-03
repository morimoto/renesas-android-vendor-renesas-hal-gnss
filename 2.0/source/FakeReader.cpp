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

#include <utility>
#include "include/FakeReader.h"
#include "include/MessageQueue.h"

static const std::string mFakeRoute = "/vendor/etc/fake_route.txt";

FakeReader::FakeReader(Transport& transport) :
    mExitThread(false),
    mTransport(transport) {
    LoadFakeTxt();
}

FakeReader::~FakeReader() {
    mFakeLocationPoints.clear();
}

RDError FakeReader::Start() {
    mExitThread = false;
    mReadingLoop = std::thread{&FakeReader::ReadingLoop, this};
    return RDError::Success;
}

RDError FakeReader::Stop() {
    mExitThread = true;

    if (mReadingLoop.joinable()) {
        mReadingLoop.join();
    }

    return RDError::Success;
}

//TODO(g.chabukiani): implement reading loop
void FakeReader::ReadingLoop() {
    auto it = mFakeLocationPoints.begin();
    MessageQueue& pipe = MessageQueue::GetInstance();

    while (!mExitThread) {
        if (mFakeLocationPoints.end() == it) {
            it = mFakeLocationPoints.begin();
        }

        auto uptrFakePoint = std::make_shared<fakeLocationPoint_t>(*it);
        pipe.Push<std::shared_ptr<fakeLocationPoint_t>>(uptrFakePoint);
        ++it;
    }
}

RDError FakeReader::LoadFakeTxt() {
    while (TError::FailedToRead != mTransport.Read<std::string>
           (mRawLocationInput)) {
        std::vector<std::string> parts;

        if (SplitLine(mRawLocationInput.back(), parts) != RDError::Success) {
            continue;
        }

        if (parts.size() != mPartsPerLine) {
            break;
        }

        mFakeLocationPoints.push_back({atof(parts[latitudeOffset].c_str()),
                        atof(parts[longitudeOffset].c_str()),
                        static_cast<float>(atof(parts[speedOffset].c_str()))
                        });
    }

    mRawLocationInput.clear();
    return mFakeLocationPoints.empty() ? \
                    RDError::InternalError : RDError::Success;
}

RDError FakeReader::SplitLine(std::string inLine,
                              std::vector<std::string>& out) {
    size_t end = 0u;
    bool separator = true;
    const std::string delimiter(",");
    out.clear();

    while (!inLine.empty()) {
        if ((end = inLine.find(delimiter)) == std::string::npos) {
            separator = false;
            end = inLine.length();
        }

        out.push_back(std::string(inLine.c_str(), end));
        inLine.erase(0, end + (separator ? 1 : 0));
    }

    return RDError::Success;
}

RDError FakeReader::SetUpNotificationCallback(cbPtr notificationCb) {
    if (nullptr != notificationCb && nullptr == mDeathNotificationCallback) {
        mDeathNotificationCallback = notificationCb;
        return RDError::Success;
    }

    return RDError::InvalidInput;
}

IReader::~IReader() {}
