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

#define LOG_TAG "GnssMeasQueue"
#define LOG_NDEBUG 1

#include <log/log.h>

#include "GnssMeasQueue.h"

GnssMeasQueue::GnssMeasQueue()
{
    ALOGV("[%s, line %d] Constructor", __func__, __LINE__);
    mQueue = std::make_unique<msgQueue>();
}

void GnssMeasQueue::push(std::shared_ptr<GnssIParser> sp)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    std::lock_guard<std::mutex> lock(mLock);
    if (mMaxSize > mQueue->size() && mState) {
        mQueue->push(sp);
        ALOGV("[%s, line %d] size = %zu", __func__, __LINE__, mQueue->size());
    }
}


std::shared_ptr<GnssIParser> GnssMeasQueue::pop()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    std::lock_guard<std::mutex> lock(mLock);
    if (mQueue->empty()) {
        ALOGV("[%s, line %d] return nullprt ", __func__, __LINE__);
        return nullptr;
    }

    ALOGV("[%s, line %d] size = %zu", __func__, __LINE__, mQueue->size());
    auto val = mQueue->front();
    mQueue->pop();

    ALOGV("[%s, line %d] Exit", __func__, __LINE__);
    return val;
}

size_t GnssMeasQueue::getSize()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    std::lock_guard<std::mutex> lock(mLock);
    ALOGV("[%s, line %d] size = %zu", __func__, __LINE__, mQueue->size());
    return mQueue->size();

}

bool GnssMeasQueue::empty()
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    std::lock_guard<std::mutex> lock(mLock);
    ALOGV("[%s, line %d] size = %zu", __func__, __LINE__, mQueue->size());
    return mQueue->empty();
}


void GnssMeasQueue::setState(const bool state)
{
    ALOGV("[%s, line %d] Entry", __func__, __LINE__);
    std::lock_guard<std::mutex> lock(mLock);
    mState = state;
    if (!state){
        while(!mQueue->empty()) {
            mQueue->pop();
            ALOGV("[%s, line %d] size = %zu", __func__, __LINE__, mQueue->size());
        }
    }
    ALOGV("[%s, line %d] Exit, size = %zu", __func__, __LINE__, mQueue->size());
}
