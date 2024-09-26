#include "Timer.h"
#include "Util.h"

namespace sylar
{

//****************************************************************************
// TimerComparator
//****************************************************************************

bool 
TimerComparator::operator()(const Timer_ptr& lhs, const Timer_ptr& rhs) const {
	if (!lhs && !rhs) return false;
	else if (!lhs && rhs) return true;
	else if (lhs && !rhs) return false;
	else {
		if (lhs->__next < rhs->__next) return true;
		if (rhs->__next < lhs->__next) return false;
		else return lhs.get() < rhs.get();
	}
}

//****************************************************************************
// Timer
//****************************************************************************

Timer::Timer(uint64_t ms, std::function<void()> cb,
			 bool recurring, TimerManager* manager)
	: __ms(ms),
	__cb(cb),
	__recurring(recurring),
	__manager(manager) {
	__next = GetCurrentMS() + __ms;
}

Timer::Timer(uint64_t next) : __next(next){}

bool Timer::cancel() {
	TimerManager::RWMutexType::WriteLock lock(__manager->__mutex);
	if (__cb) {
		__cb = nullptr;
		auto it = __manager->__timers.find(shared_from_this());
		__manager->__timers.erase(it);
		return true;
	}
	return false;
}

bool Timer::refresh() {
	TimerManager::RWMutexType::WriteLock lock(__manager->__mutex);
	if (!__cb) return false;
	auto it = __manager->__timers.find(shared_from_this());
	if (it == __manager->__timers.end()) return false;
	__manager->__timers.erase(it);
	__next = GetCurrentMS() + __ms;
	__manager->__timers.insert(shared_from_this());
	return true;
}

bool Timer::reset(uint64_t ms, bool from_now) {
	if (ms == __ms && !from_now) return true;
	TimerManager::RWMutexType::WriteLock lock(__manager->__mutex);
	if (!__cb) return false;
	auto it = __manager->__timers.find(shared_from_this());
	if (it == __manager->__timers.end()) return false;
	__manager->__timers.erase(it);
	uint64_t start = 0;
	if (from_now) start = GetCurrentMS();
	else start = __next - __ms;
	__ms = ms;
	__next = start + __ms;
	__manager->addTimer(shared_from_this(), lock);
	return true;
}

//****************************************************************************
// TimerManager
//****************************************************************************

bool TimerManager::detectClockRollover(uint64_t now_ms) {
	bool rollover = false;
	if (now_ms < __previouseTime &&
		now_ms < (__previouseTime - 60 * 60 * 1000)) {
		rollover = true;
	}
	__previouseTime = now_ms;
	return rollover;
}

void TimerManager::addTimer(Timer_ptr val, RWMutexType::WriteLock& lock) {
	auto it = __timers.insert(val).first;
	bool at_front = (it == __timers.begin()) && !__tickled;
	if (at_front) __tickled = true;
	lock.unlock();
	if (at_front) onTimerInsertedAtFront();
}

TimerManager::TimerManager() {
	__previouseTime = GetCurrentMS();
}

TimerManager::~TimerManager() {}

Timer_ptr
TimerManager::addTimer(uint64_t ms, std::function<void()> cb,
					   bool recurring) {
	Timer_ptr timer(new Timer(ms, cb, recurring, this));
	RWMutexType::WriteLock lock(__mutex);
	addTimer(timer, lock); 
	return timer;
}

static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
	std::shared_ptr<void> tmp = weak_cond.lock();
	if (tmp) cb();
}

Timer_ptr
TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb,
								std::weak_ptr<void> weak_cond, bool recurring) {
	return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recurring);
}

uint64_t TimerManager::getNextTimer() {
	RWMutexType::ReadLock lock(__mutex);
	__tickled = false;
	if (__timers.empty()) return ~0ull;

	const Timer_ptr& next = *__timers.begin();
	uint64_t now_ms = GetCurrentMS();
	if (now_ms >= next->__next) return 0;
	else return next->__next - now_ms;
}

void TimerManager::listExpiredCb(std::vector<std::function<void()>>& cbs) {
	uint64_t now_ms = GetCurrentMS();
	std::vector<Timer_ptr> expired;
	{
		RWMutexType::ReadLock lock(__mutex);
		if (__timers.empty()) return;
	}
	RWMutexType::WriteLock lock(__mutex);
	if (__timers.empty()) return;
	bool rollover = detectClockRollover(now_ms);
	if (!rollover && ((*__timers.begin())->__next > now_ms)) return;

	Timer_ptr now_timer(new Timer(now_ms));
	auto it = rollover ? __timers.end() : __timers.lower_bound(now_timer);
	while (it != __timers.end() && (*it)->__next == now_ms) {
		++it;
	}
	expired.insert(expired.begin(), __timers.begin(), it);
	__timers.erase(__timers.begin(), it);
	cbs.reserve(expired.size());

	for (auto& timer : expired) {
		cbs.push_back(timer->__cb);
		if (timer->__recurring) {
			timer->__next = now_ms + timer->__ms;
			__timers.insert(timer);
		}
		else {
			timer->__cb = nullptr;
		}
	}
}

bool TimerManager::hasTimer() {
	RWMutexType::ReadLock lock(__mutex);
	return !__timers.empty();
}

}; /* sylar */