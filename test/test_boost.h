#ifndef SYLAR_TEST_BOOST_H
#define SYLAR_TEST_BOOST_H

#include <iostream>
#include <boost/version.hpp>
#include <boost/config.hpp>
#include <boost/lexical_cast.hpp>

using std::cout;
using std::endl;

namespace Test
{

// 输出 boost 库的版本与配置基本信息
void test_boost_information() {
	cout << "boost information" << endl;

	cout << BOOST_VERSION << endl;
	cout << BOOST_LIB_VERSION << endl;
	cout << BOOST_STDLIB << endl;
	cout << BOOST_PLATFORM << endl;
	cout << BOOST_COMPILER << endl;
	cout << endl;
}

// 测试 boost 库的 lexical_cast<>() 函数的基本使用
void test_lexical_cast() {
	cout << "test lexical_cast<>()" << endl;
	const char* ptr = "32768";
	int i = boost::lexical_cast<int>(ptr);
	cout << i << endl;
}

void test_boost() {
	cout << "--------------- test boost --------------" << endl;

	test_boost_information();
	test_lexical_cast();

	cout << "----------------- test over -----------------" << endl;
	cout << endl;
}

}; /* Test */

#endif /* SYLAR_TEST_BOOST_H */
