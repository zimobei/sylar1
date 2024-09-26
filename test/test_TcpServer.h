#ifndef SYLAR_TEST_TCP_SERVER_H
#define SYLAR_TEST_TCP_SERVER_H

#include "TcpServer.h"
#include "IOManager.h"
#include "Stream.h"
#include "ByteArray.h"
#include "Log.h"
#include <iostream>
#include <memory>

using std::cout;
using std::endl;
using namespace sylar;

namespace Test
{

class EchoServer : public TcpServer {
private:
    int m_type = 0;
public:
    EchoServer(int type) : m_type(type){}
    void handleClient(Socket_ptr client) override {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "handllClient " << *client;
        SocketStream_ptr tcp(new SocketStream(client));
        ByteArray_ptr ba(new ByteArray);
        while (true) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "==== while ====";
            ba->clear();
            int rt = tcp->read(ba, 1024);
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "read rt = " << rt;

            if (rt == 0) {
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "client close: " << *client;
                break;
            }
            else if (rt < 0) {
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "client error rt = " << rt
                    << " errno = " << errno << " strerr = " << strerror(errno);
                break;
            }

            ba->setPosition(0);
            if (m_type == 1) {  //text
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "\n" << ba->toString();
            }
            else {
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "\n" << ba->toHexString();
            }
        }
    }
};

using EchoServer_ptr = std::shared_ptr<EchoServer>;

int type = 1;

void test_tcpserver_run() {
    EchoServer_ptr es(new EchoServer(type));
    auto addr = sylar::Address::LookupAny("172.23.253.254:8020");
    while (!es->bind(addr)) {
        sleep(2);
    }
    es->start();
}

void test_tcpserver() {
    cout << "--------------------------------- test TcpServer -----------------------------" << endl;
    IOManager iom(2);
    iom.schedule(test_tcpserver_run);
    cout << "--------------------------------- test over -----------------------------" << endl;
}

}; /* Test */

#endif /* SYLAR_TEST_TCP_SERVER_H */
