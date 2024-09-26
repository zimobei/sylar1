#ifndef SYLAR_TEST_IO_MANAGER_H
#define SYLAR_TEST_IO_MANAGER_H

#include "IOManager.h"
#include "Timer.h"
#include "Log.h"
#include <iostream>

using std::cout;
using std::endl;
using namespace sylar;

namespace Test
{

Timer_ptr s_timer;

void test_iomanager() {
	cout << "------------------------------------- test Iomanager ----------------------------------" << endl;
	IOManager iom(2);
	s_timer = iom.addTimer(1000, []() {
		static int i = 0;
		SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "hello timer i = " << i;
		if (++i == 3)s_timer->reset(2000, true);
		if (i == 10) s_timer->cancel();
	}, true);
	cout << "------------------------------------- test over ----------------------------------" << endl;
}

}; /* Test */

#endif /* SYLAR_TEST_IO_MANAGER_H */