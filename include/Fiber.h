//*****************************************************************************
//
//
//   ��ͷ�ļ�ʵ��Э��ģ��
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
// ǰ������
//****************************************************************************

class Fiber;
using Fiber_ptr = std::shared_ptr<Fiber>;

class Scheduler;

//****************************************************************************
// Э����
//****************************************************************************

/*!
 * @brief Э��״̬
 */
enum FiberState {
	// ��ʼ��״̬
	INIT,  
	// ��ͣ״̬
	HOLD,  
	// ִ����״̬
	EXEC,  
	// ����״̬
	TERM,  
	// ��ִ��״̬
	READY, 
	// �쳣״̬
	EXCEPT 
};

/*!
 * @brief Э����
 */
class Fiber : public std::enable_shared_from_this<Fiber> {
	friend class Scheduler;
private:
	// Э��id
	uint64_t __id = 0;
	// Э������ջ��С
	uint32_t __stack_size = 0;
	// Э��״̬
	FiberState __state = FiberState::INIT;
	// Э��������
	ucontext_t __ucontext;
	// Э������ջָ��
	void* __stack = nullptr;
	// Э�����к���
	std::function<void()> __cb;
private:
	/*!
	 * @brief �޲ι��캯�� ÿ���̵߳�һ��Э�̵Ĺ���
	 */
	Fiber();
public:
	/*!
	 * @brief ���ص�ǰЭ��
	 */
	static Fiber_ptr GetThis();

	/*!
	 * @brief ���õ�ǰ�̵߳�����Э��
	 */
	static void SetThis(Fiber* f);

	/*!
	 * @brief ��ȡ��ǰЭ�̵� id
	 */
	static uint64_t GetFiberId();

	/*!
	 * @brief ����ǰЭ���л�����̨��������ΪREADY״̬
	 */
	static void YieldToReady();

	/*!
	 * @brief ����ǰЭ���л�����̨��������ΪHOLD״̬
	 */
	static void YieldToHold();

	/*!
	 * @brief ����1��ǰЭ�̵�������
	 */
	static uint64_t TotalFibers();

	/*!
	 * @brief Э��ִ�к�����ִ����ɺ󷵻��߳���Э��
	 */
	static void MainFunc();

	/*!
	 * @brief Э��ִ�к�����ִ����ɷ��ص��̵߳���Э��
	 */
	static void CallerMainFunc();

	/*!
	 * @brief ���캯��
	 * @param cb Э��ִ�еĻص�����
	 * @param stacksize Э��ջ��С
	 */
	Fiber(std::function<void()> cb, std::size_t stacksize = 0, bool use_caller = false);

	/*!
	 * @brief ��������
	 */
	~Fiber();

	/*!
	 * @brief ����Э�� id
	 */
	uint64_t getId() const;

	/*!
	 * @brief ����Э��״̬
	 */
	FiberState getState() const;

	/*!
	 * @brief ����Э��ִ�еĻص�����,������״̬
	 */
	void reset(std::function<void()> cb);

	/*!
	 * @brief ����ǰЭ���л�������״̬
	 */
	void swapIn();

	/*!
	 * @brief ����ǰЭ���л�����̨
	 */
	void swapOut();

	/*!
	 * @brief ����ǰ�߳��л���ִ��״̬����ִ����Э��
	 */
	void call();

	/*!
	 * @brief ����ǰ�߳��л�����̨����ִ�е�ǰЭ��
	 */
	void back();
};

}; /* sylar */

#endif /* SYLAR_FIBER_H */
