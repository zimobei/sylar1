//*****************************************************************************
//
//
//   此头文件封装锁
//  
//
//*****************************************************************************


#ifndef SYLAR_MUTEX_H
#define SYLAR_MUTEX_H

#include <atomic>
#include <stdint.h>
#include <stdexcept>
#include <pthread.h>
#include <semaphore.h>
#include <boost/noncopyable.hpp>

namespace sylar {

//****************************************************************************
// 前置声明
//****************************************************************************

template<class T>
class ScopedLockImpl;

template<class T>
class ReadScopedLockImpl;

template<class T>
class WriteScopedLockImpl;

class Semaphore;

class SpinLock;

class CASLock;

class Mutex;

class NullMutex;

class RWMutex;

class NullRWMutex;

//****************************************************************************
// 局部锁的模板声明
//****************************************************************************

template<class T>
class ScopedLockImpl {
private:
    // 锁
    T& __mutex;
    // 是否已上锁
    bool __locked;
public:
    /*!
     * @brief 构造函数
     * @param mutex 锁
     */
    ScopedLockImpl(T& mutex);

    /*!
     * @brief 析构函数,自动释放锁
     */
    ~ScopedLockImpl();

    /*!
     * @brief 加锁
     */
    void lock();

    /*!
     * @brief 解锁
     */
    void unlock();
};

//****************************************************************************
// 局部读锁的模板声明
//****************************************************************************

template<class T>
class ReadScopedLockImpl {
private:
    // mutex
    T& __mutex;
    // 是否已上锁
    bool __locked;
public:
    /*!
     * @brief 构造函数
     * @param mutex 读锁
     */
    ReadScopedLockImpl(T& mutex);

    /*!
     * @brief 析构函数
     */
    ~ReadScopedLockImpl();

    /*!
     * @brief 加锁
     */
    void lock();

    /*!
     * @brief 解锁
     */
    void unlock();
};

//****************************************************************************
// 局部写锁的模板声明
//****************************************************************************

template<class T>
class WriteScopedLockImpl {
private:
    // Mutex
    T& __mutex;
    // 是否已上锁
    bool __locked;
public:
    /*!
     * @brief 构造函数
     * @param mutex 写锁
     */
    WriteScopedLockImpl(T& mutex);

    /*!
     * @brief 析构函数
     */
    ~WriteScopedLockImpl();

    /*!
     * @brief 加锁
     */
    void lock();

    /*!
     * @brief 解锁
     */
    void unlock();
};

//****************************************************************************
// 信号量类型
//****************************************************************************

class Semaphore : public boost::noncopyable{
private:
    sem_t __semaphore;
public:
    /*!
     * @brief 构造函数
     * @param count 信号量值的大小
     */
    Semaphore(uint32_t count = 0);

    /*!
     * @brief 析构函数
     */
    ~Semaphore();

    /*!
     * @brief 获取信号量
     */
    void wait();

    /*!
     * @brief  释放信号量
     */
    void notify();
};

//****************************************************************************
// 自旋锁
//****************************************************************************

class SpinLock : public boost::noncopyable {
private:
    pthread_spinlock_t __mutex;
public:    
    using Lock =  ScopedLockImpl<SpinLock>;

    /*!
     * @brief 构造函数
     */
    SpinLock();

    /*!
     * @brief 析构函数
     */
    ~SpinLock();

    /*!
     * @brief 加锁
     */
    void lock();

    /*!
     * @brief 解锁
     */
    void unlock();
};

//****************************************************************************
// 原子锁
//****************************************************************************

class CASLock : public boost::noncopyable {
private:
    /// 原子状态
    volatile std::atomic_flag __mutex;
public:
    using Lock = ScopedLockImpl<CASLock>;

    /*!
     * @brief 构造函数
     */
    CASLock();

    /*!
     * @brief 析构函数
     */
    ~CASLock();

    /*!
     * @brief 上锁
     */
    void lock();

    /*!
     * @brief 解锁
     */
    void unlock();
};

//****************************************************************************
// 互斥锁
//****************************************************************************

class Mutex : public boost::noncopyable {
private:
    // mutex
    pthread_mutex_t __mutex;
public:
    using Lock = ScopedLockImpl<Mutex>;

    /*!
     * @brief 构造函数
     */
    Mutex();

    /*!
     * @brief 析构函数
     */
    ~Mutex();

    /*!
     * @brief 加锁
     */
    void lock();

    /*!
     * @brief 解锁
     */
    void unlock();

};

//****************************************************************************
// 空锁（用于调试）
//****************************************************************************

class NullMutex : public boost::noncopyable {
public:
    using Lock = ScopedLockImpl<NullMutex>;

    /*!
     * @brief 构造函数
     */
    NullMutex();

    /*!
     * @brief 析构函数
     */
    ~NullMutex();

    /*!
     * @brief 加锁
     */
    void lock();

    /*!
     * @brief 解锁
     */
    void unlock();
};

//****************************************************************************
// 读写互斥量
//****************************************************************************

class RWMutex : public boost::noncopyable {
private:
    // 读写锁
    pthread_rwlock_t __lock;
public:
    using ReadLock = ReadScopedLockImpl<RWMutex>;
    using WriteLock = WriteScopedLockImpl<RWMutex>;;

    /*!
     * @brief 构造函数
     */
    RWMutex();

    /*!
     * @brief 析构函数
     */
    ~RWMutex();

    /*!
     * @brief 加读锁
     */
    void rdlock();

    /*!
     * @brief 加写锁
     */
    void wrlock();

    /**
     * @brief 解锁
     */
    void unlock();

};

//****************************************************************************
// 空读写锁(用于调试)
//****************************************************************************

class NullRWMutex : public boost::noncopyable {
public:
    using ReadLock = ReadScopedLockImpl<NullRWMutex>;
    using WriteLock = WriteScopedLockImpl<NullRWMutex>;;

    /*!
     * @brief 构造函数
     */
    NullRWMutex();

    /*!
     * @brief 析构函数
     */
    ~NullRWMutex();

    /*!
     * @brief 加读锁
     */
    void rdlock();

    /*!
     * @brief 加写锁
     */
    void wrlock();

    /*!
     * @brief 解锁
     */
    void unlock();
};

//****************************************************************************
// 协程信号量
//****************************************************************************


//****************************************************************************
// ScopedLockImpl<T> 的实现
//****************************************************************************

template<class T>
ScopedLockImpl<T>::ScopedLockImpl(T& mutex) : __mutex(mutex) {
    __mutex.lock();
    __locked = true;
}

template<class T>
ScopedLockImpl<T>::~ScopedLockImpl() {
    unlock();
}

template<class T>
void ScopedLockImpl<T>::lock() {
    if (!__locked) {
        __mutex.lock();
        __locked = true;
    }
}

template<class T>
void ScopedLockImpl<T>::unlock() {
    if (__locked) {
        __mutex.unlock();
        __locked = false;
    }
}

//****************************************************************************
// ReadScopedLockImpl<T> 的实现
//****************************************************************************

template<class T>
ReadScopedLockImpl<T>::ReadScopedLockImpl(T& mutex) : __mutex(mutex) {
    __mutex.rdlock();
    __locked = true;
}

template<class T>
ReadScopedLockImpl<T>::~ReadScopedLockImpl() {
    unlock();
}

template<class T>
void ReadScopedLockImpl<T>::lock() {
    if (!__locked) {
        __mutex.rdlock();
        __locked = true;
    }
}

template<class T>
void ReadScopedLockImpl<T>::unlock() {
    if (__locked) {
        __mutex.unlock();
        __locked = false;
    }
}


//****************************************************************************
// WriteScopedLockImpl<T> 的实现
//****************************************************************************

template<class T>
WriteScopedLockImpl<T>::WriteScopedLockImpl(T& mutex) : __mutex(mutex) {
    __mutex.wrlock();
    __locked = true;
}

template<class T>
WriteScopedLockImpl<T>::~WriteScopedLockImpl() {
    unlock();
}

template<class T>
void WriteScopedLockImpl<T>::lock() {
    if (!__locked) {
        __mutex.wrlock();
        __locked = true;
    }
}

template<class T>
void WriteScopedLockImpl<T>::unlock() {
    if (__locked) {
        __mutex.unlock();
        __locked = false;
    }
}



}; /* sylar */

#endif /* SYLAR_MUTEX_H */
