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

#include "include/FakeLocationProvider.h"

namespace android::hardware::gnss::V2_0::renesas {

FakeLocationProvider::FakeLocationProvider(uint32_t interval) :
    LocationProviderBase(interval),
    mBuilder(std::make_unique<FakeLocationBuilder>()) {
        LoadFakeTxt();
}

void SplitLine(std::string line, std::vector<std::string> &out) {
    int end = 0;
    bool separator = true;

    out.clear();

    while (!line.empty())
    {
        if ((end = line.find(",")) < 0) {
            separator = false;
            end = static_cast<int>(line.length());
        }
        out.push_back(std::string(line.c_str(), end));
        line.erase(0, end + (separator ? 1 : 0));
    }
}

void FakeLocationProvider::LoadFakeTxt() {
    const std::string fakeRoute = "/vendor/etc/fake_route.txt";
    std::fstream fakeStream;
    const size_t partsPerLine = 3;
    const size_t bufferSize = 128;

    fakeStream.open(fakeRoute, std::fstream::in);


    if (!fakeStream.is_open()) {
        ALOGE("Failed to open route file '%s', err=%d", fakeRoute.c_str(), errno);
        return ;
    }

    while (!fakeStream.eof()) {
        char line[bufferSize];
        fakeStream.getline(line, sizeof(line));

        std::vector<std::string> parts;
        SplitLine(std::string(line), parts);

        if (parts.size() != partsPerLine) {
            break;
        }

        fakeLocationPoint_t pt = {
            .latitude = atof(parts[0].c_str()),
            .longitude = atof(parts[1].c_str()),
            .speed = static_cast<float>(atof(parts[2].c_str()))
        };

        mFakePoints.push_back(pt);
    }
    fakeStream.close();

    ALOGI("Loaded %lu fake location point(s)", mFakePoints.size());
}

void FakeLocationProvider::Provide() {
size_t pt_idx = 0;

    while (!mThreadExit) {
        std::queue<LocationData> data;

        fakeLocationPoint_t pt_from = mFakePoints[pt_idx];
        if (++pt_idx >= mFakePoints.size()) {
            pt_idx = 0;
        }
        fakeLocationPoint_t pt_to = mFakePoints[pt_idx];

        auto error = mBuilder->Build(pt_from, pt_to, data);

        if (mEnabled && error == FLBError::SUCCESS) {
            while (!data.empty()) {
                if (mGnssCallback_1_1) {
                    ALOGV("%s, Provide fake location callback", __func__);
                    auto ret = mGnssCallback_1_1->gnssLocationCb(data.front().v1_0);

                    if (!ret.isOk()) {
                        ALOGE("%s: Unable to invoke gnssLocationCb_1_1", __func__);
                    }
                }
                if (mGnssCallback_2_0) {
                    ALOGV("%s, Provide fake location callback", __func__);
                    auto ret = mGnssCallback_2_0->gnssLocationCb_2_0(data.front());

                    if (!ret.isOk()) {
                        ALOGE("%s: Unable to invoke gnssLocationCb_2_0", __func__);
                    }
                }
                data.pop();
                usleep(mUpdateIntervalUs);
            }
        }
    }
}

} // namespace android::hardware::gnss::V2_0::renesas
