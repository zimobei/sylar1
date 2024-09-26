#ifndef SYLAR_TEST_HTTP_CONNECTION_H
#define SYLAR_TEST_HTTP_CONNECTION_H

#include "HttpConnection.h"
#include "HttpParser.h"
#include "Log.h"
#include "IOManager.h"
#include <iostream>
#include <fstream>

using namespace sylar;

namespace Test
{

void test_httpconnection_pool() {
    sylar::http::HttpConnectionPool_ptr pool(new sylar::http::HttpConnectionPool(
        "www.sylar.top", "", 80, false, 10, 1000 * 30, 5));

    sylar::IOManager::GetThis()->addTimer(1000, [pool]() {
        auto r = pool->doGet("/", 300);
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << r->toString();
    }, true);
}

void test_httpconnection_run() {
    sylar::Address_ptr addr = sylar::Address::LookupAnyIPAddress("www.sylar.top:80");
    if (!addr) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "get addr error";
        return;
    }

    sylar::Socket_ptr sock = sylar::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if (!rt) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "connect " << *addr << " failed";
        return;
    }

    sylar::http::HttpConnection_ptr conn(new sylar::http::HttpConnection(sock));
    sylar::http::HttpRequest_ptr req(new sylar::http::HttpRequest);
    req->setPath("/blog/");
    req->setHeader("host", "www.sylar.top");
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "req:" << std::endl
        << *req;

    conn->sendRequest(req);
    auto rsp = conn->recvResponse();

    if (!rsp) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "recv response error";
        return;
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "rsp:" << std::endl
        << *rsp;

    std::ofstream ofs("rsp.dat");
    ofs << *rsp;

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "=========================";

    auto r = sylar::http::HttpConnection::DoGet("http://www.sylar.top/blog/", 300);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "result=" << r->result
        << " error=" << r->error
        << " rsp=" << (r->response ? r->response->toString() : "");

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "=========================";
    test_httpconnection_pool();
}

void test_httpconnection_https() {
    auto r = sylar::http::HttpConnection::DoGet("http://www.baidu.com/", 300, {
                        {"Accept-Encoding", "gzip, deflate, br"},
                        {"Connection", "keep-alive"},
                        {"User-Agent", "curl/7.29.0"}
                                                });
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "result=" << r->result
        << " error=" << r->error
        << " rsp=" << (r->response ? r->response->toString() : "");

    //sylar::http::HttpConnectionPool_ptr pool(new sylar::http::HttpConnectionPool(
    //            "www.baidu.com", "", 80, false, 10, 1000 * 30, 5));
    auto pool = sylar::http::HttpConnectionPool::Create(
        "https://www.baidu.com", "", 10, 1000 * 30, 5);
    sylar::IOManager::GetThis()->addTimer(1000, [pool]() {
        auto r = pool->doGet("/", 3000, {
                    {"Accept-Encoding", "gzip, deflate, br"},
                    {"User-Agent", "curl/7.29.0"}
                             });
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << r->toString();
    }, true);
}

void test_httpconnection_data() {
    sylar::Address_ptr addr = sylar::Address::LookupAny("www.baidu.com:80");
    auto sock = sylar::Socket::CreateTCP(addr);

    sock->connect(addr);
    const char buff[] = "GET / HTTP/1.1\r\n"
        "connection: close\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Host: www.baidu.com\r\n\r\n";
    sock->send(buff, sizeof(buff));

    std::string line;
    line.resize(1024);

    std::ofstream ofs("http.dat", std::ios::binary);
    int total = 0;
    int len = 0;
    while ((len = sock->recv(&line[0], line.size())) > 0) {
        total += len;
        ofs.write(line.c_str(), len);
    }
    std::cout << "total: " << total << " tellp=" << ofs.tellp() << std::endl;
    ofs.flush();
}

void test_httpconnection_parser() {
    std::ifstream ifs("http.dat", std::ios::binary);
    std::string content;
    std::string line;
    line.resize(1024);

    int total = 0;
    while (!ifs.eof()) {
        ifs.read(&line[0], line.size());
        content.append(&line[0], ifs.gcount());
        total += ifs.gcount();
    }

    std::cout << "length: " << content.size() << " total: " << total << std::endl;
    sylar::http::HttpResponseParser parser;
    size_t nparse = parser.execute(&content[0], content.size(), false);
    std::cout << "finish: " << parser.isFinished() << std::endl;
    content.resize(content.size() - nparse);
    std::cout << "rsp: " << *parser.getData() << std::endl;

    auto& client_parser = parser.getParser();
    std::string body;
    int cl = 0;
    do {
        size_t nparse = parser.execute(&content[0], content.size(), true);
        std::cout << "content_len: " << client_parser.content_len
            << " left: " << content.size()
            << std::endl;
        cl += client_parser.content_len;
        content.resize(content.size() - nparse);
        body.append(content.c_str(), client_parser.content_len);
        content = content.substr(client_parser.content_len + 2);
    } while (!client_parser.chunks_done);

    std::cout << "total: " << body.size() << " content:" << cl << std::endl;

    sylar::ZlibStream_ptr stream = sylar::ZlibStream::CreateGzip(false);
    stream->write(body.c_str(), body.size());
    stream->flush();

    body = stream->getResult();

    std::ofstream ofs("http.txt");
    ofs << body;
}

void test_httpconnection() {
	sylar::IOManager iom(2);
    iom.schedule(test_httpconnection_run);
	//iom.schedule(test_httpconnection_https);
    //iom.schedule(test_httpconnection_data);
}

}; /* Test */

#endif /* SYLAR_TEST_HTTP_CONNECTION_H */
