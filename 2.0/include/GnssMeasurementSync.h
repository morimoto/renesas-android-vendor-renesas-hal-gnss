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

#include <atomic>
#include <log/log.h>

class GnssMeasurementSync {
public:
    ~GnssMeasurementSync() = default;
    static GnssMeasurementSync& GetInstance() {
        static GnssMeasurementSync instance;
        return instance;
    }

    void SetEventsToWait(int8_t numEvents);
    bool Ready();
    void NotifyEventOccured();

private:
    GnssMeasurementSync();
    GnssMeasurementSync(GnssMeasurementSync const&) = delete;
    GnssMeasurementSync& operator=(GnssMeasurementSync const&) = delete;

    std::atomic<int8_t> mEventsToWait;
};
