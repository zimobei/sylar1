//*****************************************************************************
//
//
//   此头文件实现协程模块
//  
//
//*****************************************************************************

#ifndef SYLAR_FIBER_H
#define SYLAR_FIBER_H

#include <memory>
#include <functional>
#include <ucontext.h>

namespace sylar
{

//****************************************************************************
// 前置声明
//****************************************************************************

class Fiber;
using Fiber_ptr = std::shared_ptr<Fiber>;

class Scheduler;

//****************************************************************************
// 协程类
//****************************************************************************

/*!
 * @brief 协程状态
 */
enum FiberState {
	// 初始化状态
	INIT,  
	// 暂停状态
	HOLD,  
	// 执行中状态
	EXEC,  
	// 结束状态
	TERM,  
	// 可执行状态
	READY, 
	// 异常状态
	EXCEPT 
};

/*!
 * @brief 协程类
 */
class Fiber : public std::enable_shared_from_this<Fiber> {
	friend class Scheduler;
private:
	// 协程id
	uint64_t __id = 0;
	// 协程运行栈大小
	uint32_t __stack_size = 0;
	// 协程状态
	FiberState __state = FiberState::INIT;
	// 协程上下文
	ucontext_t __ucontext;
	// 协程运行栈指针
	void* __stack = nullptr;
	// 协程运行函数
	std::function<void()> __cb;
private:
	/*!
	 * @brief 无参构造函数 每个线程第一个协程的构造
	 */
	Fiber();
public:
	/*!
	 * @brief 返回当前协程
	 */
	static Fiber_ptr GetThis();

	/*!
	 * @brief 设置当前线程的运行协程
	 */
	static void SetThis(Fiber* f);

	/*!
	 * @brief 获取当前协程的 id
	 */
	static uint64_t GetFiberId();

	/*!
	 * @brief 将当前协程切换到后台，并设置为READY状态
	 */
	static void YieldToReady();

	/*!
	 * @brief 将当前协程切换到后台，并设置为HOLD状态
	 */
	static void YieldToHold();

	/*!
	 * @brief 返回1当前协程的总数量
	 */
	static uint64_t TotalFibers();

	/*!
	 * @brief 协程执行函数，执行完成后返回线程主协程
	 */
	static void MainFunc();

	/*!
	 * @brief 协程执行函数，执行完成返回到线程调度协程
	 */
	static void CallerMainFunc();

	/*!
	 * @brief 构造函数
	 * @param cb 协程执行的回调函数
	 * @param stacksize 协程栈大小
	 */
	Fiber(std::function<void()> cb, std::size_t stacksize = 0, bool use_caller = false);

	/*!
	 * @brief 析构函数
	 */
	~Fiber();

	/*!
	 * @brief 返回协程 id
	 */
	uint64_t getId() const;

	/*!
	 * @brief 返回协程状态
	 */
	FiberState getState() const;

	/*!
	 * @brief 重置协程执行的回调函数,并重置状态
	 */
	void reset(std::function<void()> cb);

	/*!
	 * @brief 将当前协程切换到运行状态
	 */
	void swapIn();

	/*!
	 * @brief 将当前协程切换到后台
	 */
	void swapOut();

	/*!
	 * @brief 将当前线程切换到执行状态，并执行主协程
	 */
	void call();

	/*!
	 * @brief 将当前线程切换到后台，并执行当前协程
	 */
	void back();
};

}; /* sylar */

#endif /* SYLAR_FIBER_H */
