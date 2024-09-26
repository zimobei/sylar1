#ifndef SYLAR_TEST_SINGLE_H
#define SYLAR_TEST_SINGLE_H

#include "Single.h"

#include <iostream>
#include <vector>
#include <memory>

using namespace std;

namespace Test
{

void test_single() {
	cout << "-------------------- test Single ----------------------" << endl;

	cout << *sylar::Single<int>::GetInstance() << endl;
	shared_ptr<int> num = sylar::Single<int>::GetInstance();
	++(*num);
	cout << *sylar::Single<int>::GetInstance() << endl;

	cout << endl;

	shared_ptr<vector<double>> vec_ptr = sylar::Single<vector<double>>::GetInstance();
	for (auto it : *vec_ptr) {
		cout << it << "  ";
	}
	cout << endl;

	vec_ptr->emplace_back(3.14);
	vec_ptr->emplace_back(7.7);

	for (auto it : *sylar::Single<vector<double>>::GetInstance()) {
		cout << it << "  ";
	}
	cout << endl;

	cout << "-------------------- test over ----------------------" << endl;
}

}; /* Test */

#endif /* SYLAR_TEST_SINGLE_H */
