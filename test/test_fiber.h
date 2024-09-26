#ifndef SYLAR_TEST_FIBER_H
#define SYLAR_TEST_FIBER_H

#include "Fiber.h"
#include "Thread.h"
#include "Log.h"
#include <iostream>

using namespace sylar;

namespace Test
{

void run_in_fiber() {
	SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "run_in_fiber begin";
	Fiber::YieldToHold();
	SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "run_in_fiber end";
	Fiber::YieldToHold();
}

void test_fiber_fun() {
	std::cout << "fiber func" << std::endl;
	{
		Fiber::GetThis();
		Fiber_ptr fiber(new Fiber(run_in_fiber));
		SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "fiber swapin()";
		fiber->swapIn();

		SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "fiber swapin()";
		fiber->swapIn();

		SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "fiber swapin()";
		fiber->swapIn();
	}
}

void test_fiber() {
	std::cout << "------------------------- test Fiber ----------------------------" << std::endl;
	Thread::SetName("main");
	
	std::vector<Thread_ptr> thrs;
	for (size_t i = 0; i < 3; ++i) {
		Thread_ptr ptr = std::make_shared<Thread>(&test_fiber_fun, "name_" + std::to_string(i));
		thrs.push_back(ptr);
	}
	for (auto thr : thrs) {
		thr->join();
	}

	std::cout << "------------------------- test over ----------------------------" << std::endl;
}

}; /* Test */

#endif /* SYLAR_TEST_FIBER_H */
