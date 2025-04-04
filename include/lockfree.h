#pragma once

#include <atomic>
#include <vector>

// notes: if two producers or two consumers got the same slot, there will be a conflict.
//  so need to check the flat before revising the index.
//  this is more likely to happen for stacks because they both use the same index - top.
//  for queues, the index is different for producers and consumers and it can only happen
//  when many threads allocated their slots reading/writing and the pointers loop back.
//  for example
//   producer 1 got slot 0, top moved to 1
//   consumer 1 got slot 0, top moved back to 0
//   producer 2 came in, and got slot 0 as well, conflict happened if producer have not done writing yet.

class LockFreeBase
{
public:
    LockFreeBase(size_t data_size);
    virtual ~LockFreeBase() = default;
    LockFreeBase(const LockFreeBase&) = delete;
    LockFreeBase& operator=(const LockFreeBase&) = delete;
    LockFreeBase& operator=(const LockFreeBase&) volatile = delete;
    LockFreeBase(LockFreeBase&&) = delete;
    LockFreeBase& operator=(LockFreeBase&&) volatile = delete;

protected:
    virtual bool acquire_read(size_t &p) = 0;
    virtual bool acquire_write(size_t &p) = 0;
    void done_read(size_t p);
    void done_write(size_t p);
    size_t data_size;

private:
    std::vector<std::atomic_bool> valid_flags;
};

class LockFreeQueueBase : public LockFreeBase
{
public:
    LockFreeQueueBase(size_t data_size);
    
protected:
    virtual bool acquire_read(size_t &p) override;
    virtual bool acquire_write(size_t &p) override;

private:
    std::atomic<size_t> head;
    std::atomic<size_t> tail;
};

template<typename T>
class LockFreeQueue : public LockFreeQueueBase
{
private:
    size_t data_size;

    std::vector<T> data;

public:

    LockFreeQueue(size_t data_size)
        : LockFreeQueueBase(data_size)
        , data_size(data_size)
        , data(data_size)
    {}
    ~LockFreeQueue() = default;

    template<typename... Args>
    bool push(Args&&... args)
    {
        size_t p;
        if (!acquire_write(p)) return false;
        data[p] = std::forward<T>(args...);
        done_write(p);
    }

    bool pop(T& item)
    {
        size_t p;
        if (!acquire_read(p)) return false;
        item = data[p];
        done_read(p);
    }

};
