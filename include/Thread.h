//*****************************************************************************
//
//
//   ��ͷ�ļ����� pthread ��װ�Զ��� thread
//  
//
//*****************************************************************************

#ifndef SYLAR_THREAD_H
#define SYLAR_THREAD_H

#include <string>
#include <memory>
#include <functional>
#include <pthread.h>
#include <boost/noncopyable.hpp>

#include "Mutex.h"

namespace sylar{

//****************************************************************************
// ǰ������
//****************************************************************************

class Thread;
using Thread_ptr = std::shared_ptr<Thread>;

//****************************************************************************
// �̷߳�װ
//****************************************************************************

class Thread : public boost::noncopyable {
private:
    pid_t __id = -1;
    pthread_t __m_thread = 0;
    std::function<void()> __cb;
    std::string __name;
    //���ڱ�֤�̴߳����ɹ�֮����ִ�ж�Ӧ����
    Semaphore __semaphore;
public:
    /*!
     * @brief ���캯��
     * 
     * @param cb �߳�ִ�к���
     * @param name �߳�����
     */
    Thread(std::function<void()> cb, const std::string& name);

    /*!
     * @brief ��������
     */
    ~Thread();

    /*!
     * @brief ��ȡ��ǰ���߳�ָ��
     */
    static Thread* GetThis();

    /*!
     * @brief ��ȡ��ǰ���߳�����
     */
    static const std::string& GetName();

    /*!
     * @brief ���õ�ǰ���߳�����
     */
    static void SetName(const std::string& name);

    /*!
     * @brief ��ȡ�߳� ID
     */
    pid_t getId() const;

    /*!
     * @brief ��ȡ�߳�����
     */
    const std::string& getName() const;

    /*!
     * @brief �����߳�ֱ��ִ�����
     */
    void join();
private:
    static void* run(void* arg);
};

}; /* sylar */

#endif /* SYLAR_THREAD_H */
