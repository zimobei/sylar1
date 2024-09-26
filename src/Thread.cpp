#include "Log.h"
#include "Thread.h"
#include <sys/syscall.h>

namespace sylar{

//****************************************************************************
// Thread
//****************************************************************************

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";

//�����̶߳��󣬴���ص�����
Thread::Thread(std::function<void()> cb, const std::string& name)
    :__cb(cb), __name(name) 
{
    if (name.empty()) {
        __name = "UNKNOW";
    }
    int rt = pthread_create(&__m_thread, nullptr, &Thread::run, this);
    if (rt) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
            << "pthread_create thread fail, rt = " << rt
            << " "
            << "name = " << name;
        throw std::logic_error("pthread_create error");
    }
    //����δ��ɵ�ʱ��ȴ�...
    __semaphore.wait();
}

//����ʱ�̷߳��룬��֤����������
Thread::~Thread() {
    if (__m_thread) {
        pthread_detach(__m_thread);
    }
}

//��ȡ��ǰ�߳�
Thread* Thread::GetThis() {
    return t_thread;
}

//��ȡ��ǰ�߳�����
const std::string& Thread::GetName() {
    return t_thread_name;
}

//���õ�ǰ�߳�����
void Thread::SetName(const std::string& name) {
    if(name.empty()) return;
    if (t_thread) {
        t_thread->__name = name;
    }
    t_thread_name = name;
}

// ��ȡ��ǰ�̶߳���ID
pid_t Thread::getId() const{
    return __id;
}

// ��ȡ��ǰ�̶߳�������
const std::string& Thread::getName() const{
    return __name;
}

//�����̣߳���������
void Thread::join() {
    if (__m_thread) {
        int rt = pthread_join(__m_thread, nullptr);
        if (rt) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
                << "pthread_join thread fail, rt=" << rt
                << " "
                << "name = " << __name;
            throw std::logic_error("pthread_join error");
        }
        __m_thread = 0;
    }
}

void* Thread::run(void* arg){
    Thread* thread  = reinterpret_cast<Thread*>(arg);
    t_thread = thread;
    t_thread_name = thread->__name;
    thread->__id = GetThreadId();
    pthread_setname_np(pthread_self(), thread->__name.substr(0, 15).c_str());

    std::function<void()> cb;
    cb.swap(thread->__cb);

    thread->__semaphore.notify();

    cb();
    return 0;
}

}; /* sylar */