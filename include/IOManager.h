//*****************************************************************************
//
//
//   此头文件实现 IO 管理
//  
//
//*****************************************************************************


#ifndef SYLAR_IO_MANAGER_H
#define SYLAR_IO_MANAGER_H

#include "Scheduler.h"
#include "Timer.h"
#include "Mutex.h"
#include <memory>
#include <functional>
#include <atomic>
#include <vector>

namespace sylar
{

//****************************************************************************
// 前置声明
//****************************************************************************

class IOManager;
using IOManager_ptr = std::shared_ptr<IOManager>;

//****************************************************************************
// 基于 Epoll 的 IO 协程调度器
//****************************************************************************

class IOManager : public Scheduler, public TimerManager {
public:
	using RWMutexType = RWMutex;
	/*!
	 * @brief IO 事件
	 */
	enum Event {
		NONE    = 0x0,    // 无事件
		READ    = 0x1,    // 读事件（EPOLLIN）
		WRITE   = 0x2     // 写事件（EPOLLOUT）
	};
private:
	// Socket 事件上下文类
	struct FdContext {
		using MutexType = Mutex;
		/*!
		 * @brief 事件上下文类
		 */
		struct EventContext {
			Scheduler* __scheduler = nullptr;   // 事件执行的 Scheduler
			Fiber_ptr __fiber;                  // 事件协程
			std::function<void()> __cb;         // 事件的回调函数
		};

		EventContext __read;    // 读事件
		EventContext __write;   // 写事件
		int __fd = 0;           // 事件关联的句柄
		Event __events = NONE;  // 已经注册的事件
		MutexType __mutex;      // 事件的Mutex

        /*!
         * @brief 获取事件上下文类
         * @param event 事件类型
         * @return 返回对应事件的上下文
         */
        EventContext& getContext(Event event);

        /*!
         * @brief 重置事件上下文
         * @param ctx 待重置的上下文类
         */
        void resetContext(EventContext& ctx);

        /*!
         * @brief 触发事件
         * @param event 事件类型
         */
        void triggerEvent(Event event);
	};
private:
    int __epfd = 0;                                     // epoll 文件句柄  
    int __tickleFds[2];                                 // pipe 文件句柄
    std::atomic<size_t> __pendingEventCount = { 0 };    // 当前等待执行的事件数量 
    RWMutexType __mutex;                                // IOManager的Mutex
    std::vector<FdContext*> __fdContexts;               // socket事件上下文的容器

protected:
    void tickle() override;
    bool stopping() override;
    void idle() override;
    void onTimerInsertedAtFront() override;
    
    /*!
     * @brief 判断是否可以停止
     * @param timeout 最近要出发的定时器事件间隔
     * @return 返回是否可以停止
     */
    bool stopping(uint64_t& timeout);

    /*!
     * @brief 重置socket句柄上下文的容器大小
     * @param size 容量大小
     */
    void contextResize(size_t size);

public:
    /*!
     * @brief 返回当前的IOManager
     */
    static IOManager* GetThis();

    /*!
     * @brief 构造函数
     * @param threads 线程数量
     * @param use_caller 是否将调用线程包含进去
     * @param name 调度器的名称
     */
    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "");

    /*!
     * @brief 析构函数
     */
    ~IOManager();

    /*!
     * @brief 添加事件
     * @param fd socket句柄
     * @param event 事件类型
     * @param cb 事件回调函数
     * @return 添加成功返回0,失败返回-1
     */
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);

    /*!
     * @brief 删除事件
     * @param fd socket句柄
     * @param event 事件类型
     * @return 不会触发事件
     */
    bool delEvent(int fd, Event event);

    /*!
     * @brief 取消事件
     * @param fd socket句柄
     * @param event 事件类型
     * @return 如果事件存在则触发事件
     */
    bool cancelEvent(int fd, Event event);

    /*!
     * @brief 取消所有事件
     * @param fd socket句柄
     */
    bool cancelAll(int fd);
};

}; /* sylar */

#endif /* SYLAR_IO_MANAGER_H */
