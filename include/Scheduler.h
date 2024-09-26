//*****************************************************************************
//
//
//   ��ͷ�ļ���װЭ�̵�����
//  
//
//*****************************************************************************

#ifndef SYLAR_SCHEDULER_H
#define SYLAR_SCHEDULER_H

#include <atomic>
#include <memory>
#include <vector>
#include <list>
#include <string>
#include <functional>
#include <boost/noncopyable.hpp>
#include "Mutex.h"
#include "Fiber.h"
#include "Thread.h"

namespace sylar
{

//****************************************************************************
// ǰ������
//****************************************************************************

class Scheduler;
using Scheduler_ptr = std::shared_ptr<Scheduler>;

class FiberAndThread;

//****************************************************************************
// Э�̵�����
//****************************************************************************

/*!
 * @brief Э�� / ���� / �߳���
 */
class FiberAndThread {
public:
	// Э��
	Fiber_ptr __fiber;
	// Э��ִ�к���
	std::function<void()> __cb;
	// �߳� id
	int __thread_id;
public:
	/*!
	 * @brief �޲ι��캯��
	 */
	FiberAndThread();

	/*!
	 * @brief ���캯��
	 * @param f Э��
	 * @param thr �߳� id
	 */
	FiberAndThread(Fiber_ptr f, int thr);

	/*!
	 * @brief ���캯��
	 * @param f Э��ָ��
	 * @param thr �߳�id
	 */
	FiberAndThread(Fiber_ptr* f, int thr);

	/*!
	 * @brief ���캯��
	 * @param f Э��ִ�к���
	 * @param thr �߳�id
	 */
	FiberAndThread(std::function<void()> f, int thr);

	/*!
	 * @brief ���캯��
	 * @param f Э��ִ�к���ָ��
	 * @param thr �߳�id
	 */
	FiberAndThread(std::function<void()>* f, int thr);

	/*!
	 * @brief ��������
	 */
	void reset();
};

/*!
 * @brief Э�̵�����
 */
class Scheduler {
public:
	using MutexType = Mutex;
private:
	// Mutex
	MutexType __mutex;
	// �̳߳�
	std::vector<Thread_ptr> __threads;
	// ��ִ�е�Э�̶���
	std::list<FiberAndThread> __fibers;
	// use_caller Ϊ true ʱ��Ч�� ����Э��
	Fiber_ptr __root_fiber;
	// Э�̵���������
	std::string __name;
protected:
	// Э���µ��߳� id ����
	std::vector<int> __thread_ids;
	// �߳�����
	size_t __thread_count = 0;
	// �����߳�����
	std::atomic<size_t> __active_thread_count = { 0 };
	// �����߳�����
	std::atomic<size_t> __idle_thread_count = { 0 };
	// �Ƿ�����ֹͣ
	bool __is_stopping = true;
	// �Ƿ��Զ�ֹͣ
	bool __is_auto_stop = false;
	// ���߳� id ( use_caller )
	int __root_thread = 0;
private:
	/*!
	 * @brief Э�̵�������(����)
	 */
	template<class FiberOrCb>
	bool scheduleNoLock(FiberOrCb fc, int thread);
protected:
	/*!
	 * @brief ֪ͨЭ�̵�������������
	 */
	virtual void tickle();

	/*!
	 * @brief ���õ�ǰ��Э�̵�����
	 */
	void setThis();

	/*!
	 * @brief �Ƿ��п����߳�
	 */
	bool hasIdleThreads() const;

	/*!
	 * @brief Э�̵��Ⱥ���
	 */
	void run();

	/*!
	 * @brief �����Ƿ����ֹͣ
	 */
	virtual bool stopping();

	/*!
	 * @brief Э��������ɵ���ʱִ��idleЭ��
	 */
	virtual void idle();
public:
	/*!
	 * @brief ���ص�ǰЭ�̵�����
	 */
	static Scheduler* GetThis();

	/*!
	 * @brief ���ص�ǰЭ�̵������ĵ���Э��
	 */
	static Fiber* GetMainFiber();

	/*!
	 * @brief ���캯��
	 * @param threads �߳�����
	 * @param use_caller �Ƿ�ʹ�õ�ǰ�����߳�
	 * @param name Э�̵���������
	 */
	Scheduler(std::size_t threads = 1, bool use_caller = true, const std::string& name = "");

	/*!
	 * @brief ��������
	 */
	virtual ~Scheduler();

	/*!
	 * @brief ����Э�̵���������
	 */
	const std::string getName() const;

	/*!
	 * @brief ����Э�̵�����
	 */
	void start();

	/*!
	 * @brief ֹͣЭ�̵�����
	 */
	void stop();

	/*!
	 * @brief ����Э��
	 * @tparam FiberOrCb 
	 * @param fc Э�̻���
	 * @param thread Э��ִ�е��߳� id, -1 ��ʶ�����߳�
	 */
	template<class FiberOrCb>
	void schedule(FiberOrCb fc, int thread = -1);

	/*!
	 * @brief ��������Э��
	 * @param begin Э������Ŀ�ʼ
	 * @param end Э������Ľ���
	 */
	template<class InputIterator>
	void schedule(InputIterator begin, InputIterator end);

	void switchTo(int thread = -1);
	std::ostream& dump(std::ostream& os);
};

//****************************************************************************
// Э�̵�������������
//****************************************************************************

class SchedulerSwitcher : public boost::noncopyable {
private:
	Scheduler* __caller;
public:
	SchedulerSwitcher(Scheduler* target = nullptr);
	~SchedulerSwitcher();
};


//****************************************************************************
// Scheduler ģ�庯����ʵ��
//****************************************************************************

template<class FiberOrCb>
bool Scheduler::scheduleNoLock(FiberOrCb fc, int thread) {
	bool need_tickle = __fibers.empty();
	FiberAndThread  ft(fc, thread);
	if (ft.__fiber || ft.__cb) __fibers.emplace_back(ft);
	return need_tickle;
}

template<class FiberOrCb>
void Scheduler::schedule(FiberOrCb fc, int thread) {
	bool need_tickle = false;
	{
		MutexType::Lock lock(__mutex);
		need_tickle = scheduleNoLock(fc, thread);
	}
	if (need_tickle) tickle();
}

template<class InputIterator>
void Scheduler::schedule(InputIterator begin, InputIterator end) {
	bool need_tickle = false;
	{
		MutexType::Lock lock(__mutex);
		while (begin != end) {
			need_tickle = (scheduleNoLock(&*begin, -1) || need_tickle);
			++begin;
		}
	}
	if (need_tickle) tickle();
}

}; /* sylar */

#endif /* SYLAR_SCHEDULER_H */
