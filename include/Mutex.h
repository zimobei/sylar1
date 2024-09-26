//*****************************************************************************
//
//
//   ��ͷ�ļ���װ��
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
// ǰ������
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
// �ֲ�����ģ������
//****************************************************************************

template<class T>
class ScopedLockImpl {
private:
    // ��
    T& __mutex;
    // �Ƿ�������
    bool __locked;
public:
    /*!
     * @brief ���캯��
     * @param mutex ��
     */
    ScopedLockImpl(T& mutex);

    /*!
     * @brief ��������,�Զ��ͷ���
     */
    ~ScopedLockImpl();

    /*!
     * @brief ����
     */
    void lock();

    /*!
     * @brief ����
     */
    void unlock();
};

//****************************************************************************
// �ֲ�������ģ������
//****************************************************************************

template<class T>
class ReadScopedLockImpl {
private:
    // mutex
    T& __mutex;
    // �Ƿ�������
    bool __locked;
public:
    /*!
     * @brief ���캯��
     * @param mutex ����
     */
    ReadScopedLockImpl(T& mutex);

    /*!
     * @brief ��������
     */
    ~ReadScopedLockImpl();

    /*!
     * @brief ����
     */
    void lock();

    /*!
     * @brief ����
     */
    void unlock();
};

//****************************************************************************
// �ֲ�д����ģ������
//****************************************************************************

template<class T>
class WriteScopedLockImpl {
private:
    // Mutex
    T& __mutex;
    // �Ƿ�������
    bool __locked;
public:
    /*!
     * @brief ���캯��
     * @param mutex д��
     */
    WriteScopedLockImpl(T& mutex);

    /*!
     * @brief ��������
     */
    ~WriteScopedLockImpl();

    /*!
     * @brief ����
     */
    void lock();

    /*!
     * @brief ����
     */
    void unlock();
};

//****************************************************************************
// �ź�������
//****************************************************************************

class Semaphore : public boost::noncopyable{
private:
    sem_t __semaphore;
public:
    /*!
     * @brief ���캯��
     * @param count �ź���ֵ�Ĵ�С
     */
    Semaphore(uint32_t count = 0);

    /*!
     * @brief ��������
     */
    ~Semaphore();

    /*!
     * @brief ��ȡ�ź���
     */
    void wait();

    /*!
     * @brief  �ͷ��ź���
     */
    void notify();
};

//****************************************************************************
// ������
//****************************************************************************

class SpinLock : public boost::noncopyable {
private:
    pthread_spinlock_t __mutex;
public:    
    using Lock =  ScopedLockImpl<SpinLock>;

    /*!
     * @brief ���캯��
     */
    SpinLock();

    /*!
     * @brief ��������
     */
    ~SpinLock();

    /*!
     * @brief ����
     */
    void lock();

    /*!
     * @brief ����
     */
    void unlock();
};

//****************************************************************************
// ԭ����
//****************************************************************************

class CASLock : public boost::noncopyable {
private:
    /// ԭ��״̬
    volatile std::atomic_flag __mutex;
public:
    using Lock = ScopedLockImpl<CASLock>;

    /*!
     * @brief ���캯��
     */
    CASLock();

    /*!
     * @brief ��������
     */
    ~CASLock();

    /*!
     * @brief ����
     */
    void lock();

    /*!
     * @brief ����
     */
    void unlock();
};

//****************************************************************************
// ������
//****************************************************************************

class Mutex : public boost::noncopyable {
private:
    // mutex
    pthread_mutex_t __mutex;
public:
    using Lock = ScopedLockImpl<Mutex>;

    /*!
     * @brief ���캯��
     */
    Mutex();

    /*!
     * @brief ��������
     */
    ~Mutex();

    /*!
     * @brief ����
     */
    void lock();

    /*!
     * @brief ����
     */
    void unlock();

};

//****************************************************************************
// ���������ڵ��ԣ�
//****************************************************************************

class NullMutex : public boost::noncopyable {
public:
    using Lock = ScopedLockImpl<NullMutex>;

    /*!
     * @brief ���캯��
     */
    NullMutex();

    /*!
     * @brief ��������
     */
    ~NullMutex();

    /*!
     * @brief ����
     */
    void lock();

    /*!
     * @brief ����
     */
    void unlock();
};

//****************************************************************************
// ��д������
//****************************************************************************

class RWMutex : public boost::noncopyable {
private:
    // ��д��
    pthread_rwlock_t __lock;
public:
    using ReadLock = ReadScopedLockImpl<RWMutex>;
    using WriteLock = WriteScopedLockImpl<RWMutex>;;

    /*!
     * @brief ���캯��
     */
    RWMutex();

    /*!
     * @brief ��������
     */
    ~RWMutex();

    /*!
     * @brief �Ӷ���
     */
    void rdlock();

    /*!
     * @brief ��д��
     */
    void wrlock();

    /**
     * @brief ����
     */
    void unlock();

};

//****************************************************************************
// �ն�д��(���ڵ���)
//****************************************************************************

class NullRWMutex : public boost::noncopyable {
public:
    using ReadLock = ReadScopedLockImpl<NullRWMutex>;
    using WriteLock = WriteScopedLockImpl<NullRWMutex>;;

    /*!
     * @brief ���캯��
     */
    NullRWMutex();

    /*!
     * @brief ��������
     */
    ~NullRWMutex();

    /*!
     * @brief �Ӷ���
     */
    void rdlock();

    /*!
     * @brief ��д��
     */
    void wrlock();

    /*!
     * @brief ����
     */
    void unlock();
};

//****************************************************************************
// Э���ź���
//****************************************************************************


//****************************************************************************
// ScopedLockImpl<T> ��ʵ��
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
// ReadScopedLockImpl<T> ��ʵ��
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
// WriteScopedLockImpl<T> ��ʵ��
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
