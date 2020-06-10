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
#define LOG_NDEBUG 1
#define LOG_TAG "GnssRenesasMeasurementBuilder"
#include <log/log.h>

#include <chrono>

#include "include/MeasurementBuilder.h"

#include <utils/SystemClock.h>

using namespace std::chrono;
namespace android::hardware::gnss::V2_0::renesas {

static constexpr uint32_t numParsersExpected = 4;
static constexpr size_t msgQueTimeout = 1000;
static constexpr double getParsersTimeout = 2000;

MeasurementBuilder::MeasurementBuilder() :
    mMsgQueue(MessageQueue::GetInstance()),
    mCondVar(mMsgQueue.GetConditionVariable<MBType>()) {}

void MeasurementBuilder::AddElapsedRealtime(GnssData& outData) {
    outData.elapsedRealtime.flags = ElapsedRealtimeFlags::HAS_TIMESTAMP_NS |
                                     ElapsedRealtimeFlags::HAS_TIME_UNCERTAINTY_NS;
    outData.elapsedRealtime.timestampNs =
        static_cast<uint64_t>(::android::elapsedRealtimeNano());
    outData.elapsedRealtime.timeUncertaintyNs = 0;
}

MBError MeasurementBuilder::Build(GnssData& outData) {
    auto parserMap = std::make_unique<MBParserMap> ();

    if (auto res = GetParsers(*parserMap); res != MBError::SUCCESS) {
        ALOGV("%s: can't get valid GNSS measurement", __func__);
        return res;
    }

    MBType parser;

    for (const auto& elem : *parserMap) {
        parser = elem.second;

        if (!parser->IsValid()) {
            return MBError::INVALID;
        }

        parser->GetData(&outData);
    }

    AddElapsedRealtime(outData);
    return MBError::SUCCESS;
}

MBError MeasurementBuilder::GetParsers(MBParserMap& outMap) {
    MBType parser;
    auto start = steady_clock::now();
    double uptime;

    while (outMap.size() != numParsersExpected || mMsgQueue.GetSize<MBType>() > 0) {
        parser = mMsgQueue.Pop<MBType>();

        if (parser == nullptr) {
            std::unique_lock<std::mutex> lock(mLock);
            mCondVar.wait_for(lock, milliseconds{msgQueTimeout},
                              [&] { return !mMsgQueue.Empty<MBType>(); });
        } else {
            outMap.insert_or_assign(parser->GetMsgType(), parser);
        }

        uptime =
            duration_cast<milliseconds>(steady_clock::now() - start).count();
        if (uptime >= getParsersTimeout){
            break;
        }
    }

    if (outMap.size() != numParsersExpected) {
        return outMap.size() ? MBError::INCOMPLETE : MBError::EMPTY;
    }

    return MBError::SUCCESS;
}
}
