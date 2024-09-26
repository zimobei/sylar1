#ifndef SYLAR_TEST_HOOK_H
#define SYLAR_TEST_HOOK_H

#include "Hook.h"
#include "Log.h"
#include "IOManager.h"
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>

using std::cout;
using std::endl;
using std::string;
using namespace sylar;

namespace Test
{

void test_hook_sleep() {
    sylar::IOManager iom(1);
    iom.schedule([]() {
        sleep(2);
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "sleep 2";
    });

    iom.schedule([]() {
        sleep(3);
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "sleep 3";
    });
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "test_hook_sleep";
}

void test_hook_sock() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "115.239.210.27", &addr.sin_addr.s_addr);

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "begin connect";
    int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "connect rt = " << rt << " errno = " << errno;

    if (rt) {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "send rt = " << rt << " errno = " << errno;

    if (rt <= 0) {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "recv rt = " << rt << " errno = " << errno;

    if (rt <= 0) {
        return;
    }

    buff.resize(rt);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << buff;
}

void test_hook() {
	cout << "------------------------- test Hook -------------------------------" << endl;
    test_hook_sleep();
    IOManager iom;
    iom.schedule(test_hook_sock);
	cout << "------------------------- test over -------------------------------" << endl;
}

}; /* Test*/

#endif /* SYLAR_TEST_HOOK_H */
