//*****************************************************************************
//
//
//   ��ͷ�ļ�ʵ�� IO ����
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
// ǰ������
//****************************************************************************

class IOManager;
using IOManager_ptr = std::shared_ptr<IOManager>;

//****************************************************************************
// ���� Epoll �� IO Э�̵�����
//****************************************************************************

class IOManager : public Scheduler, public TimerManager {
public:
	using RWMutexType = RWMutex;
	/*!
	 * @brief IO �¼�
	 */
	enum Event {
		NONE    = 0x0,    // ���¼�
		READ    = 0x1,    // ���¼���EPOLLIN��
		WRITE   = 0x2     // д�¼���EPOLLOUT��
	};
private:
	// Socket �¼���������
	struct FdContext {
		using MutexType = Mutex;
		/*!
		 * @brief �¼���������
		 */
		struct EventContext {
			Scheduler* __scheduler = nullptr;   // �¼�ִ�е� Scheduler
			Fiber_ptr __fiber;                  // �¼�Э��
			std::function<void()> __cb;         // �¼��Ļص�����
		};

		EventContext __read;    // ���¼�
		EventContext __write;   // д�¼�
		int __fd = 0;           // �¼������ľ��
		Event __events = NONE;  // �Ѿ�ע����¼�
		MutexType __mutex;      // �¼���Mutex

        /*!
         * @brief ��ȡ�¼���������
         * @param event �¼�����
         * @return ���ض�Ӧ�¼���������
         */
        EventContext& getContext(Event event);

        /*!
         * @brief �����¼�������
         * @param ctx �����õ���������
         */
        void resetContext(EventContext& ctx);

        /*!
         * @brief �����¼�
         * @param event �¼�����
         */
        void triggerEvent(Event event);
	};
private:
    int __epfd = 0;                                     // epoll �ļ����  
    int __tickleFds[2];                                 // pipe �ļ����
    std::atomic<size_t> __pendingEventCount = { 0 };    // ��ǰ�ȴ�ִ�е��¼����� 
    RWMutexType __mutex;                                // IOManager��Mutex
    std::vector<FdContext*> __fdContexts;               // socket�¼������ĵ�����

protected:
    void tickle() override;
    bool stopping() override;
    void idle() override;
    void onTimerInsertedAtFront() override;
    
    /*!
     * @brief �ж��Ƿ����ֹͣ
     * @param timeout ���Ҫ�����Ķ�ʱ���¼����
     * @return �����Ƿ����ֹͣ
     */
    bool stopping(uint64_t& timeout);

    /*!
     * @brief ����socket��������ĵ�������С
     * @param size ������С
     */
    void contextResize(size_t size);

public:
    /*!
     * @brief ���ص�ǰ��IOManager
     */
    static IOManager* GetThis();

    /*!
     * @brief ���캯��
     * @param threads �߳�����
     * @param use_caller �Ƿ񽫵����̰߳�����ȥ
     * @param name ������������
     */
    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "");

    /*!
     * @brief ��������
     */
    ~IOManager();

    /*!
     * @brief ����¼�
     * @param fd socket���
     * @param event �¼�����
     * @param cb �¼��ص�����
     * @return ��ӳɹ�����0,ʧ�ܷ���-1
     */
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);

    /*!
     * @brief ɾ���¼�
     * @param fd socket���
     * @param event �¼�����
     * @return ���ᴥ���¼�
     */
    bool delEvent(int fd, Event event);

    /*!
     * @brief ȡ���¼�
     * @param fd socket���
     * @param event �¼�����
     * @return ����¼������򴥷��¼�
     */
    bool cancelEvent(int fd, Event event);

    /*!
     * @brief ȡ�������¼�
     * @param fd socket���
     */
    bool cancelAll(int fd);
};

}; /* sylar */

#endif /* SYLAR_IO_MANAGER_H */
