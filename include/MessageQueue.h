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

#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <condition_variable>
#include <list>
#include <mutex>
#include <queue>

#include <log/log.h>

/**
 * @brief Message Queue
 */
class MessageQueue {
public:
    ~MessageQueue() = default;

    /**
     * @brief Get the Instance object
     *
     * @return MessageQueue&
     */
    static MessageQueue& GetInstance();

    /**
     * @brief Push
     *
     * @tparam T
     * @param in
     */
    template <typename T>
    void Push(T in);

    /**
     * @brief Pop
     *
     * @tparam T
     * @return T
     */
    template <typename T>
    T Pop();

    /**
     * @brief Front
     *
     * @tparam T
     * @return T
     */
    template <typename T>
    T Front();

    // TODO(g.chabukiani): review this design better
    /**
     * @brief Empty
     *
     * @tparam T
     * @return true
     * @return false
     */
    template <typename T>
    bool Empty();

    /**
     * @brief Get the Size object
     *
     * @tparam T
     * @return size_t
     */
    template <typename T>
    size_t GetSize();

    /**
     * @brief Clear message queue
     *
     * @tparam T
     */
    template <typename T>
    void Clear();

    /**
     * @brief Get the Condition Variable object
     *
     * @tparam T
     * @return std::condition_variable&
     */
    template <typename T>
    std::condition_variable& GetConditionVariable();

private:
    MessageQueue() = default;
    MessageQueue(MessageQueue const&) = delete;
    MessageQueue& operator=(MessageQueue const&) = delete;

    template <typename T>
    static std::queue<T, std::list<T>> mQueue;

    template <typename T>
    struct Sync {
        std::mutex mLock;
    };

    template <typename T>
    static Sync<T> locker;

    template <typename T>
    struct Condintion {
        std::condition_variable cv;
    };

    template <typename T>
    static Condintion<T> mConditionVariable;

    const size_t mMaxSize = 128;
};

template <typename T>
MessageQueue::Condintion<T> MessageQueue::mConditionVariable;

template <typename T>
MessageQueue::Sync<T> MessageQueue::locker;

template <typename T>
std::queue<T, std::list<T>> MessageQueue::mQueue;

template <typename T>
void MessageQueue::Push(T in) {
    std::lock_guard<std::mutex> lock(locker<T>.mLock);
    ALOGV("GnssRenesasQueue [%s]", __PRETTY_FUNCTION__);

    if (mMaxSize <= mQueue<T>.size()) {
        ALOGV("GnssRenesasQueue [%s] mMaxSize <= size(), %zu",
            __PRETTY_FUNCTION__, mQueue<T>.size());
        mQueue<T>.pop();
    }

    mQueue<T>.push(std::move(in));
    auto size = mQueue<T>.size();
    ALOGV("GnssRenesasQueue [%s], size = %zu", __PRETTY_FUNCTION__, size);
    mConditionVariable<T>.cv.notify_all();
}

template <typename T>
T MessageQueue::Pop() {
    std::lock_guard<std::mutex> lock(locker<T>.mLock);
    ALOGV("GnssRenesasQueue [%s]", __PRETTY_FUNCTION__);

    if (mQueue<T>.empty()) {
        ALOGV("GnssRenesasQueue [%s] empty", __PRETTY_FUNCTION__);
        return nullptr;
    }

    auto size = mQueue<T>.size();
    ALOGV("GnssRenesasQueue [%s], size = %zu", __PRETTY_FUNCTION__, size);
    auto val = mQueue<T>.front();
    mQueue<T>.pop();
    return val;
}


template <typename T>
T MessageQueue::Front() {
    std::lock_guard<std::mutex> lock(locker<T>.mLock);
    ALOGV("GnssRenesasQueue [%s]", __PRETTY_FUNCTION__);

    if (mQueue<T>.empty()) {
        ALOGV("GnssRenesasQueue [%s] empty", __PRETTY_FUNCTION__);
        return nullptr;
    }

    auto size = mQueue<T>.size();
    ALOGV("GnssRenesasQueue [%s], size = %zu", __PRETTY_FUNCTION__, size);
    auto val = mQueue<T>.front();
    return val;
}

template <typename T>
size_t MessageQueue::GetSize() {
    std::lock_guard<std::mutex> lock(locker<T>.mLock);
    return mQueue<T>.size();
}

template <typename T>
bool MessageQueue::Empty() {
    std::lock_guard<std::mutex> lock(locker<T>.mLock);
    auto size = mQueue<T>.size();
    ALOGV("GnssRenesasQueue [%s], size = %zu", __PRETTY_FUNCTION__, size);
    return mQueue<T>.empty();
}

template <typename T>
void MessageQueue::Clear() {
    if (!mQueue<T>.empty()) {
        std::queue<T, std::list<T>> empty;
        mQueue<T>.swap(empty);
    }
}

template <typename T>
std::condition_variable& MessageQueue::GetConditionVariable() {
    return mConditionVariable<T>.cv;
}

#endif // MESSAGEQUEUE_H
