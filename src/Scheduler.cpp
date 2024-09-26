#include "Scheduler.h"
#include "Log.h"
#include "Util.h"
#include "Macro.h"

namespace sylar
{

//****************************************************************************
// Э�̵����������ڲ�����
//****************************************************************************

// ָ��Э�̵�������ָ��
static thread_local Scheduler* t_scheduler = nullptr;
// ��Э��
static thread_local Fiber* t_scheduler_fiber = nullptr;

//****************************************************************************
// FiberAndThread
//****************************************************************************

FiberAndThread::FiberAndThread()
	: __thread_id(-1){}

FiberAndThread::FiberAndThread(Fiber_ptr f, int thr) 
	: __fiber(f), __thread_id(thr){}

FiberAndThread::FiberAndThread(Fiber_ptr* f, int thr) 
	: __thread_id(thr){
	__fiber.swap(*f);
}

FiberAndThread::FiberAndThread(std::function<void()> f, int thr) 
	: __cb(f), __thread_id(thr) {}

FiberAndThread::FiberAndThread(std::function<void()>* f, int thr) 
	: __thread_id(thr){
	__cb.swap(*f);
}

void FiberAndThread::reset() {
	__fiber = nullptr;
	__cb = nullptr;
	__thread_id = -1;
}

//****************************************************************************
// Scheduler
//****************************************************************************

void Scheduler::tickle() {
	SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "tickle";
}

void Scheduler::setThis() {
	t_scheduler = this;
}

bool Scheduler::hasIdleThreads() const {
	return __idle_thread_count > 0;
}

void Scheduler::run() {
	SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << __name << " run ";
	setThis();
	if (GetThreadId() != __root_thread) {
		t_scheduler_fiber = Fiber::GetThis().get();
	}

	Fiber_ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
	Fiber_ptr cb_fiber;
	FiberAndThread ft;

	while (true) {
		ft.reset();
		bool tickle_me = false;
		bool is_active = false;
		{
			MutexType::Lock lock(__mutex);
			auto it = __fibers.begin();
			while (it != __fibers.end()) {
				if (it->__thread_id != -1 && it->__thread_id != GetThreadId()) {
					++it;
					tickle_me = true;
					continue;
				}
				SYLAR_ASSERT(it->__fiber || it->__cb);
				if (it->__fiber && it->__fiber->getState() == FiberState::EXEC) {
					++it;
					continue;
				}
				
				ft = *it;
				__fibers.erase(it++);
				++__active_thread_count;
				is_active = true;
				break;
			}
			tickle_me |= it != __fibers.end();
		}

		if (tickle_me) tickle();

		if (ft.__fiber &&
			(ft.__fiber->getState() != FiberState::TERM &&
			 ft.__fiber->getState() != FiberState::EXCEPT)) {
			ft.__fiber->swapIn();
			--__active_thread_count;

			if (ft.__fiber->getState() == FiberState::READY) {
				schedule(ft.__fiber);
			}
			else if (ft.__fiber->getState() != FiberState::TERM &&
					 ft.__fiber->getState() != FiberState::EXCEPT) {
				ft.__fiber->__state = FiberState::HOLD;
			}
			ft.reset();
		}
		else if (ft.__cb) {
			if (cb_fiber) {
				cb_fiber->reset(ft.__cb);
			}
			else {
				cb_fiber.reset(new Fiber(ft.__cb));
			}
			ft.reset();

			cb_fiber->swapIn();
			--__active_thread_count;
			
			if (cb_fiber->getState() == FiberState::READY) {
				schedule(cb_fiber);
				cb_fiber.reset();
			}
			else if (cb_fiber->getState() == FiberState::EXCEPT ||
					 cb_fiber->getState() == FiberState::TERM) {
				cb_fiber->reset(nullptr);
			}
			else {
				cb_fiber->__state = FiberState::HOLD;
				cb_fiber.reset();
			}
		}
		else {
			if (is_active) {
				--__active_thread_count;
				continue;
			}
			if (idle_fiber->getState() == FiberState::TERM) {
				SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "idle fiber term";
				break;
			}

			++__idle_thread_count;
			idle_fiber->swapIn();
			--__idle_thread_count;

			if (idle_fiber->getState() != FiberState::TERM &&
				idle_fiber->getState() != FiberState::EXCEPT) {
				idle_fiber->__state = FiberState::HOLD;
			}
		}
	}
}

bool Scheduler::stopping() {
	MutexType::Lock lock(__mutex);
	return __is_auto_stop &&
		   __is_stopping && 
		   __fibers.empty() && 
		   (__active_thread_count == 0);
}

void Scheduler::idle() {
	SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "idle";
	while (!stopping()) {
		Fiber::YieldToHold();
	}
}

Scheduler* Scheduler::GetThis() {
	return t_scheduler;
}

Fiber* Scheduler::GetMainFiber() {
	return t_scheduler_fiber;
}

Scheduler::Scheduler(std::size_t threads, bool use_caller, const std::string& name) 
	: __name(name)
{
	SYLAR_ASSERT(threads > 0);

	if (use_caller) {
		// use_caller �Ὣ��ǰ���� Э�̵����� ���̴߳�ŵ�Э�̵�������
		// ���ԣ���Ҫ������ thread ������һ
		--threads;  
		// ���û��Э�������һ��Э��
		Fiber::GetThis(); 

		// ֻ�е�ǰ�߳��в�����Э�̵��������� GetThis() == nullptr ����ʱ��
		// �ſ����ڵ�ǰ�߳��д���Э�̵�����
		SYLAR_ASSERT(GetThis() == nullptr); 
		// ����Э�̵�����ָ��
		t_scheduler = this;

		// ����һ���� run ������Э��
		__root_fiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
		// �����߳�����
		Thread::SetName(__name);

		t_scheduler_fiber = __root_fiber.get();
		__root_thread = GetThreadId();
		__thread_ids.push_back(__root_thread);
	}
	else {
		__root_thread = -1;
	}
	__thread_count = threads;
}

Scheduler::~Scheduler() {
	SYLAR_ASSERT(__is_stopping);
	if (GetThis() == this) {
		t_scheduler = nullptr;
	}
}

const std::string Scheduler::getName() const {
	return __name;
}

void Scheduler::start() {
	// ����
	MutexType::Lock lock(__mutex);
	// �Ѿ����������������
	if (!__is_stopping) return;
	// ����Э�̵�����״̬Ϊ������
	__is_stopping = false;
	// ��ʱЭ�̵������е��̳߳�Ӧ��Ϊ��
	SYLAR_ASSERT(__threads.empty());
	// �����Ӧ�������̣߳�����ִ�к���
	__threads.resize(__thread_count);
	for (std::size_t i = 0; i < __thread_count; ++i) {
		__threads[i].reset(
			new Thread(
				std::bind(&Scheduler::run, this),
				__name + "_" + std::to_string(i)));
		__thread_ids.push_back(__threads[i]->getId());
	}
	// lock.unlock();
}

void Scheduler::stop() {
	__is_auto_stop = true;
	if (__root_fiber &&
		__thread_count == 0 &&
		(__root_fiber->getState() == FiberState::TERM ||
		 __root_fiber->getState() == FiberState::INIT)) {
		SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << this << " stopped";
		__is_stopping = true;

		if (stopping()) return;
	}

	// use_caller �߳�
	if (__root_thread != -1) {
		SYLAR_ASSERT(GetThis() == this);
	}
	// �� use_caller �߳�
	else {
		SYLAR_ASSERT(GetThis() != this);
	}

	__is_stopping = true;
	for (std::size_t i = 0; i < __thread_count; ++i) {
		tickle();
	}

	if (__root_fiber) tickle();

	if (__root_fiber && !stopping()) __root_fiber->call();

	std::vector<Thread_ptr> thrs;
	{
		MutexType::Lock lock(__mutex);
		thrs.swap(__threads);
	}
	for (auto& i : thrs) {
		i->join();
	}
}
 
void Scheduler::switchTo(int thread) {
	SYLAR_ASSERT(Scheduler::GetThis() != nullptr);
	if (Scheduler::GetThis() == this &&
		(thread == -1 || thread == GetThreadId())) {
		return;
	}
	schedule(Fiber::GetThis(), thread);
	Fiber::YieldToHold();
}

std::ostream& Scheduler::dump(std::ostream& os) {
	os << "[Scheduler name=" << __name
		<< " size=" << __thread_count
		<< " active_count=" << __active_thread_count
		<< " idle_count=" << __idle_thread_count
		<< " stopping=" << __is_stopping
		<< " ]" << std::endl << "    ";
	for (std::size_t i = 0; i < __thread_ids.size(); ++i) {
		if (i) os << ", ";
		os << __thread_ids[i];
	}
	return os;
}

//****************************************************************************
// SchedulerSwitcher
//****************************************************************************

SchedulerSwitcher::SchedulerSwitcher(Scheduler* target) {
	__caller = Scheduler::GetThis();
	if (target) target->switchTo();
}

SchedulerSwitcher::~SchedulerSwitcher() {
	if (__caller) __caller->switchTo();
}

}; /* sylar */