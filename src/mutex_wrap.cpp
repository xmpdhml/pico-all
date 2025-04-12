#include "mutex_wrap.h"

Mutex::Mutex() {
    mutex_init(&_mutex);
}

Mutex::~Mutex() {
}

void Mutex::enter() {
    mutex_enter_blocking(&_mutex);
}

void Mutex::exit() {
    mutex_exit(&_mutex);
}

Mutex::Scoped::Scoped(Mutex& mutex) : _mutex(mutex) {
    _mutex.enter();
}

Mutex::Scoped::~Scoped() {
    _mutex.exit();
}
