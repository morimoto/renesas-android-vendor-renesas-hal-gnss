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

#ifndef __GNSSMEASTOLOCSYNC_H__
#define __GNSSMEASTOLOCSYNC_H__

#include <atomic>
#include <memory>
#include <chrono>
#include <condition_variable>
#include <log/log.h>


class GnssMeasToLocSync
{
public:
    ~GnssMeasToLocSync() {}

    /*!
     * \brief getInstance - provide an instance of the single object, create if there is no object
     * \return reference - to the sync object
     */
    static GnssMeasToLocSync& getInstance();

    /*!
     * \brief WaitAndSend - wait for measurements to be sent enough, before locationCb provide location
     * \return true if ready to send, otherwise false
     */
    bool WaitToSend();

    /*!
     * \brief UpdateStatus - increase or decrese number of events to wait
     * \param numEvents - signed value of number of events
     */
    void UpdateStatus(int8_t numEvents);

    /*!
     * \brief SetEventsToWait - set value of number of events to wait
     * \param numEvents - number of events to wait
     */
    void SetEventsToWait(int8_t numEvents);

private:
    GnssMeasToLocSync();
    GnssMeasToLocSync(GnssMeasToLocSync const&) = delete;
    GnssMeasToLocSync &operator=(GnssMeasToLocSync const&) = delete;

    std::atomic<int8_t> mEventsToWait;
};

#endif // __GNSSMEASTOLOCSYNC_H__
