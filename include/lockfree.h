#pragma once

#include <atomic>
#include <vector>
#include "pico/types.h"


class LockFreeQueueBase
{
public:
    LockFreeQueueBase(uint data_size);
    ~LockFreeQueueBase() = default;
    LockFreeQueueBase(const LockFreeQueueBase&) = delete;
    LockFreeQueueBase& operator=(const LockFreeQueueBase&) = delete;
    LockFreeQueueBase& operator=(const LockFreeQueueBase&) volatile = delete;
    LockFreeQueueBase(LockFreeQueueBase&&) = delete;
    LockFreeQueueBase& operator=(LockFreeQueueBase&&) volatile = delete;

protected:
    bool acquire_read(uint &p);
    bool acquire_write(uint &p);
    void done_read(uint p);
    void done_write(uint p);

private:
    uint data_size;
    std::vector<std::atomic_bool> used;
    std::atomic<uint> head;
    std::atomic<uint> tail;
};

template<typename T>
class LockFreeQueue : public LockFreeQueueBase
{
private:
    uint data_size;

    std::vector<T> data;

public:

    LockFreeQueue(uint data_size)
        : LockFreeQueueBase(data_size)
        , data_size(data_size)
        , data(data_size)
    {}
    ~LockFreeQueue() = default;

    template<typename... Args>
    bool push(Args&&... args)
    {
        uint p;
        if (!acquire_write(p)) return false;
        data[p] = std::forward<T>(args...);
        done_write(p);
    }

    bool pop(T& item)
    {
        uint p;
        if (!acquire_read(p)) return false;
        item = data[p];
        done_read(p);
    }

};
