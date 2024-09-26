//*****************************************************************************
//
//
//   定时器
//  
//
//*****************************************************************************

#ifndef SYLAR_TIMER_H
#define SYLAR_TIMER_H

#include "Mutex.h"
#include <memory>
#include <vector>
#include <set>
#include <functional>

namespace sylar
{

//****************************************************************************
// 前置声明
//****************************************************************************

class Timer;
using Timer_ptr = std::shared_ptr<Timer>;

class TimerManager;

struct TimerComparator;

//****************************************************************************
// 定时器比较器
//****************************************************************************

struct TimerComparator {
    /*!
     * @brief 按执行时间比较定时器智能指针的大小
     */
    bool operator()(const Timer_ptr& lhs, const Timer_ptr& rhs) const;
};


//****************************************************************************
// 定时器
//****************************************************************************

class Timer : public std::enable_shared_from_this<Timer> {
	friend class TimerManager;
    friend struct TimerComparator;
private:  
    bool __recurring = false;           // 是否循环定时器
    uint64_t __ms = 0;                  // 执行周期
    uint64_t __next = 0;                // 精确的执行时间
    std::function<void()> __cb;         // 回调函数
    TimerManager* __manager = nullptr;  // 定时器管理器
private:
    /*!
     * @brief 构造函数
     * @param ms 定时器执行间隔时间
     * @param cb 回调函数
     * @param recurring 是否循环
     * @param manager 定时器管理器
     */
    Timer(uint64_t ms, std::function<void()> cb,
          bool recurring, TimerManager* manager);

    /*!
     * @brief  构造函数
     * @param next 执行的时间戳(毫秒)
     */
    Timer(uint64_t next);
public:
    /*!
     * @brief 取消定时器
     */
    bool cancel();

    /*!
     * @brief 刷新设置定时器的执行时间
     */
    bool refresh();

    /*!
     * @brief 重置定时器时间
     * @param ms 定时器执行间隔时间(毫秒)
     * @param from_now 是否从当前时间开始计算
     */
    bool reset(uint64_t ms, bool from_now);
};

//****************************************************************************
// 时间器管理器
//****************************************************************************

class TimerManager {
    friend class Timer;
public:
    using RWMutexType = RWMutex;
private:
    RWMutexType __mutex;                                // Mutex
    std::set<Timer_ptr, TimerComparator> __timers;      // 定时器集合
    bool __tickled = false;                             // 是否触发onTimerInsertedAtFront
    uint64_t __previouseTime = 0;                       // 上次执行时间
private:
    /*!
     * @brief 检测服务器时间是否被调后了
     */
    bool detectClockRollover(uint64_t now_ms);
protected:
    /*!
     * @brief 当有新的定时器插入到定时器的首部,执行该函数
     */
    virtual void onTimerInsertedAtFront() = 0;

    /*!
     * @brief 将定时器添加到管理器中
     */
    void addTimer(Timer_ptr val, RWMutexType::WriteLock& lock);
public:
    /*!
     * @brief 构造函数
     */
    TimerManager();

    /*!
     * @brief 析构函数
     */
    virtual ~TimerManager();

    /*!
     * @brief 添加定时器
     * @param ms 定时器执行间隔时间
     * @param cb 定时器回调函数
     * @param recurring 是否循环定时器
     */
    Timer_ptr 
    addTimer(uint64_t ms, std::function<void()> cb, 
             bool recurring = false);

    /*!
     * @brief 添加条件定时器
     * @param ms 定时器执行间隔时间
     * @param cb 定时器回调函数
     * @param weak_cond 条件
     * @param recurring 是否循环
     */
    Timer_ptr 
    addConditionTimer(uint64_t ms, std::function<void()> cb, 
                      std::weak_ptr<void> weak_cond, bool recurring = false);

    /*!
     * @brief 到最近一个定时器执行的时间间隔(毫秒)
     */
    uint64_t getNextTimer();

    /*!
     * @brief 获取需要执行的定时器的回调函数列表
     * @param cbs 回调函数数组
     */
    void listExpiredCb(std::vector<std::function<void()> >& cbs);

    /*!
     * @brief 是否有定时器
     */
    bool hasTimer();
};

}; /* sylar */

#endif /* SYLAR_TIMER_H */
