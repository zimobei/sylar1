#ifndef SYLAR_TEST_HTTP_H
#define SYLAR_TEST_HTTP_H

#include "Http.h"
#include "Log.h"
#include <iostream>
using std::cout;
using std::endl;
using namespace sylar;

namespace Test
{

void test_http_request() {
    sylar::http::HttpRequest_ptr req(new sylar::http::HttpRequest);
    req->setHeader("host", "www.sylar.top");
    req->setBody("hello sylar");
    req->dump(std::cout) << std::endl;
}

void test_http_response() {
    sylar::http::HttpResponse_ptr rsp(new sylar::http::HttpResponse);
    rsp->setHeader("X-X", "sylar");
    rsp->setBody("hello sylar");
    rsp->setStatus((sylar::http::HttpStatus)400);
    rsp->setClose(false);

    rsp->dump(std::cout) << std::endl;
}

void test_http() {
	cout << "---------------------- test http -----------------------" << endl;
    test_http_request();
    test_http_response();
	cout << "---------------------- test over -----------------------" << endl;
}

};

#endif /* SYLAR_TEST_HTTP_H */
