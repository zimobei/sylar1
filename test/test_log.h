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
        logger->getName(),              //��־������
        LogLevel::INFO,		            //��־����
        __FILE__, 			            //�ļ�����
        __LINE__, 			            //�к�
        1234567, 			            //��������ʱ��
        GetThreadId(),		            //�߳�ID
        "default_thread",               //�߳� name
        3, 					            //Э��ID
        time(0)				            //��ǰʱ��
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
