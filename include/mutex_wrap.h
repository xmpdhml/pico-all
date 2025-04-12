#ifndef PICO_CRITICAL_SECTION_WRAPPER_H
#define PICO_CRITICAL_SECTION_WRAPPER_H

#include "pico/mutex.h"

class Mutex {
private:
    mutex_t _mutex;

public:
    Mutex();

    ~Mutex();

    void enter();

    void exit();

    class Scoped {
    private:
        Mutex& _mutex;
    public:
        Scoped(Mutex& mutex);

        ~Scoped();

        operator bool() {
            return true;
        }

        Scoped(const Scoped&) = delete;
        Scoped& operator=(const Scoped&) = delete;
    };

    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
};

#define MUTEX_LOCK(mutex) if (Mutex::Scoped _scoped_mutex = mutex)

#endif // PICO_CRITICAL_SECTION_WRAPPER_H
