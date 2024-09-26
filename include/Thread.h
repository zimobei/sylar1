//*****************************************************************************
//
//
//   此头文件根据 pthread 封装自定义 thread
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
// 前置声明
//****************************************************************************

class Thread;
using Thread_ptr = std::shared_ptr<Thread>;

//****************************************************************************
// 线程封装
//****************************************************************************

class Thread : public boost::noncopyable {
private:
    pid_t __id = -1;
    pthread_t __m_thread = 0;
    std::function<void()> __cb;
    std::string __name;
    //用于保证线程创建成功之后再执行对应方法
    Semaphore __semaphore;
public:
    /*!
     * @brief 构造函数
     * 
     * @param cb 线程执行函数
     * @param name 线程名称
     */
    Thread(std::function<void()> cb, const std::string& name);

    /*!
     * @brief 析构函数
     */
    ~Thread();

    /*!
     * @brief 获取当前的线程指针
     */
    static Thread* GetThis();

    /*!
     * @brief 获取当前的线程名称
     */
    static const std::string& GetName();

    /*!
     * @brief 设置当前的线程名称
     */
    static void SetName(const std::string& name);

    /*!
     * @brief 获取线程 ID
     */
    pid_t getId() const;

    /*!
     * @brief 获取线程名称
     */
    const std::string& getName() const;

    /*!
     * @brief 阻塞线程直至执行完成
     */
    void join();
private:
    static void* run(void* arg);
};

}; /* sylar */

#endif /* SYLAR_THREAD_H */
