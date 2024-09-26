//*****************************************************************************
//
//
//   此头文件封装协程调度器
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
// 前置声明
//****************************************************************************

class Scheduler;
using Scheduler_ptr = std::shared_ptr<Scheduler>;

class FiberAndThread;

//****************************************************************************
// 协程调度器
//****************************************************************************

/*!
 * @brief 协程 / 函数 / 线程组
 */
class FiberAndThread {
public:
	// 协程
	Fiber_ptr __fiber;
	// 协程执行函数
	std::function<void()> __cb;
	// 线程 id
	int __thread_id;
public:
	/*!
	 * @brief 无参构造函数
	 */
	FiberAndThread();

	/*!
	 * @brief 构造函数
	 * @param f 协程
	 * @param thr 线程 id
	 */
	FiberAndThread(Fiber_ptr f, int thr);

	/*!
	 * @brief 构造函数
	 * @param f 协程指针
	 * @param thr 线程id
	 */
	FiberAndThread(Fiber_ptr* f, int thr);

	/*!
	 * @brief 构造函数
	 * @param f 协程执行函数
	 * @param thr 线程id
	 */
	FiberAndThread(std::function<void()> f, int thr);

	/*!
	 * @brief 构造函数
	 * @param f 协程执行函数指针
	 * @param thr 线程id
	 */
	FiberAndThread(std::function<void()>* f, int thr);

	/*!
	 * @brief 重置数据
	 */
	void reset();
};

/*!
 * @brief 协程调度器
 */
class Scheduler {
public:
	using MutexType = Mutex;
private:
	// Mutex
	MutexType __mutex;
	// 线程池
	std::vector<Thread_ptr> __threads;
	// 待执行的协程队列
	std::list<FiberAndThread> __fibers;
	// use_caller 为 true 时有效， 调度协程
	Fiber_ptr __root_fiber;
	// 协程调度器名称
	std::string __name;
protected:
	// 协程下的线程 id 数组
	std::vector<int> __thread_ids;
	// 线程数量
	size_t __thread_count = 0;
	// 工作线程数量
	std::atomic<size_t> __active_thread_count = { 0 };
	// 空闲线程数量
	std::atomic<size_t> __idle_thread_count = { 0 };
	// 是否正在停止
	bool __is_stopping = true;
	// 是否自动停止
	bool __is_auto_stop = false;
	// 主线程 id ( use_caller )
	int __root_thread = 0;
private:
	/*!
	 * @brief 协程调度启动(无锁)
	 */
	template<class FiberOrCb>
	bool scheduleNoLock(FiberOrCb fc, int thread);
protected:
	/*!
	 * @brief 通知协程调度器有任务了
	 */
	virtual void tickle();

	/*!
	 * @brief 设置当前的协程调度器
	 */
	void setThis();

	/*!
	 * @brief 是否有空闲线程
	 */
	bool hasIdleThreads() const;

	/*!
	 * @brief 协程调度函数
	 */
	void run();

	/*!
	 * @brief 返回是否可以停止
	 */
	virtual bool stopping();

	/*!
	 * @brief 协程无任务可调度时执行idle协程
	 */
	virtual void idle();
public:
	/*!
	 * @brief 返回当前协程调度器
	 */
	static Scheduler* GetThis();

	/*!
	 * @brief 返回当前协程调度器的调度协程
	 */
	static Fiber* GetMainFiber();

	/*!
	 * @brief 构造函数
	 * @param threads 线程数量
	 * @param use_caller 是否使用当前调用线程
	 * @param name 协程调度器名称
	 */
	Scheduler(std::size_t threads = 1, bool use_caller = true, const std::string& name = "");

	/*!
	 * @brief 析构函数
	 */
	virtual ~Scheduler();

	/*!
	 * @brief 返回协程调度器名称
	 */
	const std::string getName() const;

	/*!
	 * @brief 启动协程调度器
	 */
	void start();

	/*!
	 * @brief 停止协程调度器
	 */
	void stop();

	/*!
	 * @brief 调度协程
	 * @tparam FiberOrCb 
	 * @param fc 协程或函数
	 * @param thread 协程执行的线程 id, -1 标识任意线程
	 */
	template<class FiberOrCb>
	void schedule(FiberOrCb fc, int thread = -1);

	/*!
	 * @brief 批量调度协程
	 * @param begin 协程数组的开始
	 * @param end 协程数组的结束
	 */
	template<class InputIterator>
	void schedule(InputIterator begin, InputIterator end);

	void switchTo(int thread = -1);
	std::ostream& dump(std::ostream& os);
};

//****************************************************************************
// 协程调度器的派生类
//****************************************************************************

class SchedulerSwitcher : public boost::noncopyable {
private:
	Scheduler* __caller;
public:
	SchedulerSwitcher(Scheduler* target = nullptr);
	~SchedulerSwitcher();
};


//****************************************************************************
// Scheduler 模板函数的实现
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
