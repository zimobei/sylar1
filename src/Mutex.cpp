#include "Mutex.h"

namespace sylar {

//****************************************************************************
// Semaphore
//****************************************************************************

Semaphore::Semaphore(uint32_t count) {
    if (sem_init(&__semaphore, 0, count)) {
        throw std::logic_error("sem_init error");
    }
}

Semaphore::~Semaphore() {
    sem_destroy(&__semaphore);
}

void Semaphore::wait() {
    if (sem_wait(&__semaphore)) {
        throw std::logic_error("sem_wait error");
    }
}

void Semaphore::notify() {
    if (sem_post(&__semaphore)) {
        throw std::logic_error("sem_post error");
    }
}

//****************************************************************************
// SpinLock
//****************************************************************************

SpinLock::SpinLock() {
    pthread_spin_init(&__mutex, 0);
}

SpinLock::~SpinLock() {
    pthread_spin_destroy(&__mutex);
}

void SpinLock::lock() {
    pthread_spin_lock(&__mutex);
}

void SpinLock::unlock() {
    pthread_spin_unlock(&__mutex);
}

//****************************************************************************
// CASLock
//****************************************************************************

CASLock::CASLock() {
    __mutex.clear();
}

CASLock::~CASLock() {}

void CASLock::lock() {
    while (std::atomic_flag_test_and_set_explicit(&__mutex, std::memory_order_acquire));
}

void CASLock::unlock() {
    std::atomic_flag_clear_explicit(&__mutex, std::memory_order_release);
}

//****************************************************************************
// Mutex
//****************************************************************************

Mutex::Mutex() {
    pthread_mutex_init(&__mutex, nullptr);
}

Mutex::~Mutex() {
    pthread_mutex_destroy(&__mutex);
}

void Mutex::lock() {
    pthread_mutex_lock(&__mutex);
}

void Mutex::unlock() {
    pthread_mutex_unlock(&__mutex);
}

//****************************************************************************
// NullMutex
//****************************************************************************

NullMutex::NullMutex() {}

NullMutex::~NullMutex() {}

void NullMutex::lock() {}

void NullMutex::unlock() {}

//****************************************************************************
// RWMutex
//****************************************************************************

RWMutex::RWMutex() {
    pthread_rwlock_init(&__lock, nullptr);
}

RWMutex::~RWMutex() {
    pthread_rwlock_destroy(&__lock);
}

void RWMutex::rdlock() {
    pthread_rwlock_rdlock(&__lock);
}

void RWMutex::wrlock() {
    pthread_rwlock_wrlock(&__lock);
}

void RWMutex::unlock() {
    pthread_rwlock_unlock(&__lock);
}

//****************************************************************************
// NullRWMutex
//****************************************************************************

NullRWMutex::NullRWMutex() {}

NullRWMutex::~NullRWMutex() {}

void NullRWMutex::rdlock() {}

void NullRWMutex::wrlock() {}

void NullRWMutex::unlock() {}


}; /* sylar */