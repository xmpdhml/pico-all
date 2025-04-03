#include "lockfree.h"

LockFreeQueueBase::LockFreeQueueBase(size_t data_size)
    : data_size(data_size),
      used(data_size),
      head(0),
      tail(1)
{
    for (size_t i = 0; i < data_size; ++i)
    {
        used[i].store(false, std::memory_order_release);
    }
}

bool LockFreeQueueBase::acquire_read(size_t &p)
{
    while (true)
    {
        size_t h, nh;
        h = head.load(std::memory_order_acquire);
        nh = (h + 1) % data_size;
        if (nh == tail.load(std::memory_order_acquire)) return false;

        if (head.compare_exchange_weak(h, nh, std::memory_order_acq_rel) == false)
        {
            // acquired, wait for data
            while (!used[h].load(std::memory_order_acquire));
            p = h;
            return true;
        }
    }
}

void LockFreeQueueBase::done_read(size_t p)
{
    used[p].store(false, std::memory_order_release);
}

bool LockFreeQueueBase::acquire_write(size_t &p)
{
    while (true)
    {
        size_t t, nt;
        t = tail.load(std::memory_order_acquire);
        if (t == head.load(std::memory_order_acquire)) return false;
        nt = (t + 1) % data_size;

        if (tail.compare_exchange_weak(t, nt, std::memory_order_acq_rel) == false)
        {
            // acquired, wait for data
            while (used[t].load(std::memory_order_acquire));
            p = t;
            return true;
        }
    }
}