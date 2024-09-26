#ifndef SYLAR_TEST_SCHEDULER_H
#define SYLAR_TEST_SCHEDULER_H

#include <iostream>
#include "Scheduler.h"
#include "Log.h"
#include "Util.h"

using namespace sylar;

namespace Test
{

void test_scheduler_func() {
    static int s_count = 5;
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "test in fiber s_count = " << s_count;

    sleep(1);
    if (--s_count >= 0) {
        Scheduler::GetThis()->schedule(&test_scheduler_func, GetThreadId());
    }
}

void test_scheduler() {
    std::cout << "------------------------------ test Scheduler ------------------------" << std::endl;
    Scheduler sc(3);
    sc.start();
    sleep(2);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "schedule";
    sc.schedule(&test_scheduler_func);
    sc.stop();
    std::cout << "------------------------------ test over ------------------------" << std::endl;
}

}; /* Test */

#endif /* SYLAR_TEST_SCHEDULER_H */