//*****************************************************************************
//
//
//   ��ʱ��
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
// ǰ������
//****************************************************************************

class Timer;
using Timer_ptr = std::shared_ptr<Timer>;

class TimerManager;

struct TimerComparator;

//****************************************************************************
// ��ʱ���Ƚ���
//****************************************************************************

struct TimerComparator {
    /*!
     * @brief ��ִ��ʱ��Ƚ϶�ʱ������ָ��Ĵ�С
     */
    bool operator()(const Timer_ptr& lhs, const Timer_ptr& rhs) const;
};


//****************************************************************************
// ��ʱ��
//****************************************************************************

class Timer : public std::enable_shared_from_this<Timer> {
	friend class TimerManager;
    friend struct TimerComparator;
private:  
    bool __recurring = false;           // �Ƿ�ѭ����ʱ��
    uint64_t __ms = 0;                  // ִ������
    uint64_t __next = 0;                // ��ȷ��ִ��ʱ��
    std::function<void()> __cb;         // �ص�����
    TimerManager* __manager = nullptr;  // ��ʱ��������
private:
    /*!
     * @brief ���캯��
     * @param ms ��ʱ��ִ�м��ʱ��
     * @param cb �ص�����
     * @param recurring �Ƿ�ѭ��
     * @param manager ��ʱ��������
     */
    Timer(uint64_t ms, std::function<void()> cb,
          bool recurring, TimerManager* manager);

    /*!
     * @brief  ���캯��
     * @param next ִ�е�ʱ���(����)
     */
    Timer(uint64_t next);
public:
    /*!
     * @brief ȡ����ʱ��
     */
    bool cancel();

    /*!
     * @brief ˢ�����ö�ʱ����ִ��ʱ��
     */
    bool refresh();

    /*!
     * @brief ���ö�ʱ��ʱ��
     * @param ms ��ʱ��ִ�м��ʱ��(����)
     * @param from_now �Ƿ�ӵ�ǰʱ�俪ʼ����
     */
    bool reset(uint64_t ms, bool from_now);
};

//****************************************************************************
// ʱ����������
//****************************************************************************

class TimerManager {
    friend class Timer;
public:
    using RWMutexType = RWMutex;
private:
    RWMutexType __mutex;                                // Mutex
    std::set<Timer_ptr, TimerComparator> __timers;      // ��ʱ������
    bool __tickled = false;                             // �Ƿ񴥷�onTimerInsertedAtFront
    uint64_t __previouseTime = 0;                       // �ϴ�ִ��ʱ��
private:
    /*!
     * @brief ��������ʱ���Ƿ񱻵�����
     */
    bool detectClockRollover(uint64_t now_ms);
protected:
    /*!
     * @brief �����µĶ�ʱ�����뵽��ʱ�����ײ�,ִ�иú���
     */
    virtual void onTimerInsertedAtFront() = 0;

    /*!
     * @brief ����ʱ����ӵ���������
     */
    void addTimer(Timer_ptr val, RWMutexType::WriteLock& lock);
public:
    /*!
     * @brief ���캯��
     */
    TimerManager();

    /*!
     * @brief ��������
     */
    virtual ~TimerManager();

    /*!
     * @brief ��Ӷ�ʱ��
     * @param ms ��ʱ��ִ�м��ʱ��
     * @param cb ��ʱ���ص�����
     * @param recurring �Ƿ�ѭ����ʱ��
     */
    Timer_ptr 
    addTimer(uint64_t ms, std::function<void()> cb, 
             bool recurring = false);

    /*!
     * @brief ���������ʱ��
     * @param ms ��ʱ��ִ�м��ʱ��
     * @param cb ��ʱ���ص�����
     * @param weak_cond ����
     * @param recurring �Ƿ�ѭ��
     */
    Timer_ptr 
    addConditionTimer(uint64_t ms, std::function<void()> cb, 
                      std::weak_ptr<void> weak_cond, bool recurring = false);

    /*!
     * @brief �����һ����ʱ��ִ�е�ʱ����(����)
     */
    uint64_t getNextTimer();

    /*!
     * @brief ��ȡ��Ҫִ�еĶ�ʱ���Ļص������б�
     * @param cbs �ص���������
     */
    void listExpiredCb(std::vector<std::function<void()> >& cbs);

    /*!
     * @brief �Ƿ��ж�ʱ��
     */
    bool hasTimer();
};

}; /* sylar */

#endif /* SYLAR_TIMER_H */
