#ifndef SYLAR_TEST_LOG_H
#define SYLAR_TEST_LOG_H

#include "Log.h"

#include <iostream>
#include <thread>

using std::cout;
using std::endl;
using namespace sylar;

namespace Test
{

void test_log() {
    std::cout << "-------------- test log.h -------------------" << std::endl;

    Logger_ptr logger(new Logger("XYZ"));
    LogEvent_ptr event(new LogEvent(
        logger->getName(),              //日志器名称
        LogLevel::INFO,		            //日志级别
        __FILE__, 			            //文件名称
        __LINE__, 			            //行号
        1234567, 			            //程序运行时间
        GetThreadId(),		            //线程ID
        "default_thread",               //线程 name
        3, 					            //协程ID
        time(0)				            //当前时间
    ));

    LogFormatter_ptr formatter(new LogFormatter());
    StdOutLogAppender_ptr stdApp(new StdOutLogAppender());
    stdApp->setFormatter(formatter);
    logger->addAppender(stdApp);
    SYLAR_LOG_INFO(logger) << "Hello Info";
    cout << "---------------- test over ---------------------" << endl;
    cout << endl;
}

}; /* Test */

#endif /* SYLAR_TEST_LOG_H */
