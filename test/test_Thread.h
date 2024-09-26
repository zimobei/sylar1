#ifndef SYLAR_TEST_THREAD_H
#define SYLAR_TEST_THREAD_H

#include "Log.h"
#include "Thread.h"

#include <iostream>
#include <vector>

using namespace sylar;

namespace Test
{

int count = 0;
Mutex s_mutex;

// �ص�����1
void test_thread_func1(){
    for(std::size_t i = 0; i < 5; ++i){
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "XXXXXXXXXXXXXXXXXXXX";
    }
}

// �ص�����2
void test_thread_func2(){
    for(std::size_t i = 0; i < 5; ++i){
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "====================";
    }
}

// �ص�����3
void test_thread_func3(){
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
        << "\n"
        << "name = " << Thread::GetName()
        << " "
        << "this.name = " << Thread::GetThis()->getName()
        << " "
        << "id = " << GetThreadId()
        << " "
        << "this.id = " << Thread::GetThis()->getId();
    for (int i = 0; i < 100000; ++i) {
        Mutex::Lock lock(s_mutex);
        ++count;
    }
}

void test_thread(){
    std::cout << "------------------- test Thread -----------------------" << std::endl;
    // ���� vector �����߳�
    std::vector<Thread_ptr> vecs;
    // �����߳�
    Thread_ptr thread1 = std::make_shared<Thread>(&test_thread_func1, "name_1");
    Thread_ptr thread2 = std::make_shared<Thread>(&test_thread_func2, "name_2");
    Thread_ptr thread3 = std::make_shared<Thread>(&test_thread_func3, "name_3");
    vecs.push_back(thread1);
    vecs.push_back(thread2);
    vecs.push_back(thread3);
    // �����߳�
    for (std::size_t i = 0; i < vecs.size(); ++i) {
        vecs[i]->join();
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "count = " << count;
    std::cout << "------------------- test over -----------------------" << std::endl;
}

}; /* Test*/



#endif /* SYLAR_TEST_THREAD_H */
