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

#include <list>
#include <queue>
#include <mutex>
#include <condition_variable>

//TODO(g.chabukiani): add doxygen, check all over the project
class MessageQueue {
public:
    ~MessageQueue() = default;
    static MessageQueue& GetInstance();

    template<typename T>
    void Push(T in);

    template<typename T>
    T Pop();

    // TODO(g.chabukiani): review this design better
    template<typename T>
    bool Empty();

    template<typename T>
    size_t GetSize();

    template<typename T>
    void Clear();

    template<typename T>
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

    if (mMaxSize > mQueue<T>.size()) {
        ALOGV("GnssRenesasQueue [%s] mMaxSize > size(), %zu",
            __PRETTY_FUNCTION__, mQueue<T>.size());
        mQueue<T>.push(std::move(in));
    }

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
    if (!mQueue<T>.empty()){
        std::queue<T, std::list<T>> empty;
        mQueue<T>.swap(empty);
    }
}

template <typename T>
std::condition_variable& MessageQueue::GetConditionVariable() {
    return mConditionVariable<T>.cv;
}
