#ifndef SYLAR_TEST_SOCKET_H
#define SYLAR_TEST_SOCKET_H

#include "Socket.h"
#include "IOManager.h"
#include "Log.h"
#include <iostream>

using std::cout;
using std::endl;
using namespace sylar;

namespace Test
{

void test_socket_func1() {
    IPAddress_ptr addr = Address::LookupAnyIPAddress("www.baidu.com");
    if (addr) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "get address: " << addr->toString();
    }
    else {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "get address fail";
        return;
    }

    Socket_ptr sock = Socket::CreateTCP(addr);
    addr->setPort(80);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "addr=" << addr->toString();
    if (!sock->connect(addr)) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "connect " << addr->toString() << " fail";
        return;
    }
    else {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if (rt <= 0) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if (rt <= 0) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << buffs;
}

void test_socket_func2() {
    IPAddress_ptr addr = Address::LookupAnyIPAddress("www.baidu.com:80");
    if (addr) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "get address: " << addr->toString();
    }
    else {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "get address fail";
        return;
    }

    Socket_ptr sock = Socket::CreateTCP(addr);
    if (!sock->connect(addr)) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "connect " << addr->toString() << " fail";
        return;
    }
    else {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "connect " << addr->toString() << " connected";
    }

    uint64_t ts = GetCurrentUS();
    for (size_t i = 0; i < 10000000000ul; ++i) {
        if (int err = sock->getError()) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "err=" << err << " errstr=" << strerror(err);
            break;
        }

        static int batch = 10000000;
        if (i && (i % batch) == 0) {
            uint64_t ts2 = GetCurrentUS();
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "i=" << i << " used: " << ((ts2 - ts) * 1.0 / batch) << " us";
            ts = ts2;
        }
    }
}

void test_socket() {
	cout << "------------------------------------- test socket ----------------------------" << endl;

	IOManager iom;
	iom.schedule(&test_socket_func1);
    // iom.schedule(&test_socket_func2);

	cout << "------------------------------------- test over ----------------------------" << endl;
}

}; /* Test */


#endif /* SYLAR_TEST_SOCKET_H */
