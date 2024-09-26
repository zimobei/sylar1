#ifndef SYLAR_TEST_UTIL_H
#define SYLAR_TEST_UTIL_H

#include "Util.h"
#include <iostream>

using namespace sylar;

namespace Test
{

void test_util() {
	std::cout << "---------------------- test Util -------------------------" << std::endl;
	std::cout << "GetThreadId() : " << GetThreadId() << std::endl;
	std::cout << BacktraceToString(10, 0, "Backtrace information : ") << std::endl;
	std::cout << "---------------------- test over -------------------------" << std::endl;
}

}; /* Test */



#endif // !SYLAR_TEST_UTIL_H
