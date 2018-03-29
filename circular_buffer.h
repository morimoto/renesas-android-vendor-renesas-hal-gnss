#include <cstdio>

#include <memory>
#include <mutex>
#include <string.h>

#include <cutils/log.h>

template <class T>
class CircularBuffer {
public:
    explicit CircularBuffer(size_t size, size_t element_size) :
        buf_(std::unique_ptr<T[]>(new T[size])),
        size_(size),
        element_size_(element_size)
    {
    }

    void put(void* data)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        memcpy(&buf_[head_], data, element_size_);
        head_ = (head_ + 1) % size_;

        if(head_ == tail_)
        {
            tail_ = (tail_ + 1) % size_;
        }
    }

    T* get(void)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if(empty())
        {
            return nullptr;
        }

        //Read data and advance the tail (we now have a free space)
        auto val = &buf_[tail_];
        tail_ = (tail_ + 1) % size_;

        return val;
    }

    void reset(void)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        head_ = tail_;
    }

    bool empty(void) const
    {
        //if head and tail are equal, we are empty
        return head_ == tail_;
    }

    bool full(void) const
    {
        //If tail is ahead the head by 1, we are full
        return ((head_ + 1) % size_) == tail_;
    }

    size_t size(void) const
    {
        return size_ - 1;
    }

private:
    std::mutex mutex_;
    std::unique_ptr<T[]> buf_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_;
    size_t element_size_;
};
