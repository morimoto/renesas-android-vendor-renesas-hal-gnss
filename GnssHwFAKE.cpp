/*
 * Copyright (C) 2017 GlobalLogic
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

#define LOG_TAG "GnssRenesasHAL"

#include <unistd.h>
#include <inttypes.h>
#include <math.h>
#include <fstream>
#include <string>

#include <log/log.h>
#include <cutils/properties.h>

#include <android/hardware/gnss/1.0/IGnss.h>

#include "GnssHw.h"

#define EARTH_RADIUS            6373000 // in meters
#define PI                      3.141592653589793

GnssHwFAKE::GnssHwFAKE(void)
{
}

GnssHwFAKE::~GnssHwFAKE(void)
{
}

void GnssHwFAKE::SplitLine(std::string line, std::vector<std::string> &out)
{
    int end = 0;
    bool separator = true;

    out.clear();

    while (!line.empty())
    {
        if ((end = line.find(",")) < 0) {
            separator = false;
            end = (int)line.length();
        }
        out.push_back(std::string(line.c_str(), end));
        line.erase(0, end + (separator ? 1 : 0));
    }
}

bool GnssHwFAKE::start(void)
{
    std::string fake_route = "/vendor/etc/fake_route.txt";
    std::fstream file;

    ALOGD("Start FAKE");

    file.open(fake_route, std::fstream::in);

    if (!file.is_open()) {
        ALOGE("Failed to open route file '%s', err=%d", fake_route.c_str(), errno);
        return false;
    }

    while (!file.eof()) {
        char line[128];
        file.getline(line, sizeof(line));

        std::vector<std::string> parts;
        SplitLine(std::string(line), parts);

        if (parts.size() != 3) {
            break;
        }

        FakeLocationPoint pt = {
            .latitude = atof(parts[0].c_str()),
            .longitude = atof(parts[1].c_str()),
            .speed = static_cast<float>(atof(parts[2].c_str()))
        };

        mFakePoints.push_back(pt);
    }

    file.close();

    ALOGI("Loaded %lu fake location point(s)", mFakePoints.size());

    return (mFakePoints.size() > 1);
}

bool GnssHwFAKE::stop(void)
{
    ALOGD("Stop FAKE");

    mFakePoints.clear();
    return true;
}

void GnssHwFAKE::GnssHwHandleThread(void)
{
    ALOGD("GnssFakeHandleThread() ->");

    size_t pt_idx = 0;
    GnssLocation location;

    location.horizontalAccuracyMeters = 1.f;
    location.gnssLocationFlags = static_cast<uint16_t>(
            GnssLocationFlags::HAS_SPEED | GnssLocationFlags::HAS_HORIZONTAL_ACCURACY |
            GnssLocationFlags::HAS_LAT_LONG | GnssLocationFlags::HAS_ALTITUDE |
            GnssLocationFlags::HAS_LAT_LONG | GnssLocationFlags::HAS_BEARING);

    while (!mThreadExit)
    {
        if (mGnssCb == nullptr || mFakePoints.size() < 2) {
            usleep(1000000); /* Sleeping */
            continue;
        }

        FakeLocationPoint &pt_from = mFakePoints[pt_idx];

        if (++pt_idx >= mFakePoints.size()) {
            pt_idx = 0;
        }

        FakeLocationPoint &pt_to = mFakePoints[pt_idx];

        /* ---------------------------------------------------------- */
        location.speedMetersPerSec = pt_from.speed;
        location.latitudeDegrees = pt_from.latitude;
        location.longitudeDegrees = pt_from.longitude;

        // Calculating distance in meters between points
        float dlon = (pt_to.longitude - pt_from.longitude) * PI / 180;
        float dlat = (pt_to.latitude - pt_from.latitude) * PI / 180;
        float a = pow((sin(dlat / 2)), 2) + cos(pt_from.latitude * PI / 180) * cos(pt_to.latitude * PI / 180) * pow((sin(dlon / 2)), 2);
        float c = 2 * atan2(sqrt(a), sqrt(1 - a));
        float d = EARTH_RADIUS * c; // Distance
        float t = d / location.speedMetersPerSec; // Time in seconds to reach last point
        float latStep = (pt_to.latitude - pt_from.latitude) / t;
        float lonStep = (pt_to.longitude - pt_from.longitude) / t; // lat lon step for next reporting point

        // Calculating bearing between points
        float X = cos(pt_to.latitude * PI / 180) * sin(dlon);
        float Y = cos(pt_from.latitude * PI / 180) * sin(pt_to.latitude * PI / 180) -
                sin(pt_from.latitude * PI / 180) * cos(pt_to.latitude * PI / 180) * cos(dlon);
        float bearing = atan2(X, Y) * 180 / PI;

        location.bearingDegrees = bearing;

        time_t tm = {};
        (void)tm;
        for (int i = (int)t; i >= 0; i--) {
            if (i == 0) {
                location.latitudeDegrees = pt_to.latitude;
                location.longitudeDegrees = pt_to.longitude;
                location.speedMetersPerSec = d - ((int)t) * pt_from.speed;
                if (location.speedMetersPerSec < pt_from.speed / 5) {
                    break;
                }
            } else {
                location.latitudeDegrees += latStep;
                location.longitudeDegrees += lonStep;
            }

            location.timestamp = time(NULL) * 1000; // timestamp of the event in milliseconds, time(NULL) returns seconds, therefore we need to convert

            auto ret = mGnssCb->gnssLocationCb(location);
            if (!ret.isOk()) {
                ALOGE("%s: Unable to invoke gnssLocationCb", __func__);
            }

            usleep(requestedUpdateIntervalUs);
        }
    }

    ALOGD("GnssFakeHandleThread() <-");
}

bool GnssHwFAKE::setUpdatePeriod(int periodMs)
{
    requestedUpdateIntervalUs = periodMs * 1000;
    return true;
}
