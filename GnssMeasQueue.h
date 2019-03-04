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

#ifndef __GNSSMEASQUEUE_H__
#define __GNSSMEASQUEUE_H__

#include <list>
#include <queue>
#include <mutex>

#include "GnssIParser.h"


class GnssMeasQueue
{
public:
    ~GnssMeasQueue() {}

    /*!
     * \brief getInstance - provide an instance of the single object, create if there is no object
     * \brief  this is FIFO queue
     * \return reference - to the queue
     */
    static GnssMeasQueue& getInstance() {
        static GnssMeasQueue instance;
        return instance;
    }

    /*!
     * \brief push -  add an object to the end of the queue
     * \brief works only if mState is set as true
     * \param object - a pointer to the new object
     */
    void push(std::shared_ptr<GnssIParser> object);

    /*!
     * \brief pop - provide the first(front) object and remove it from the queue
     * \return - a pointer to the object
     */
    std::shared_ptr<GnssIParser> pop();

    /*!
     * \brief empty - denote if the queue is empty or not
     * \return true if empty, otherwise false
     */
    bool empty();

    /*!
     * \brief getSize - provide the number of element in the queue
     * \return the number of elements in the queue
     */
    size_t getSize();

    /*!
     * \brief setState - denote if the queue is on or off
     * \brief set mState member
     * \brief do not add any objects if the queue is off
     * \brief clean up the queue if going to turn off
     * \param state -- true to turn on, false to turn off
     */
    void setState(const bool state);

private:
    GnssMeasQueue();
    GnssMeasQueue(GnssMeasQueue const&) = delete;
    GnssMeasQueue &operator=(GnssMeasQueue const&) = delete;

    const size_t mMaxSize = 100;
    bool mState = false;

    typedef std::queue<std::shared_ptr<GnssIParser>, std::list<std::shared_ptr<GnssIParser>>> msgQueue;
    std::unique_ptr<msgQueue> mQueue;
    std::mutex mLock;
};

#endif // __GNSSMEASQUEUE_H__
