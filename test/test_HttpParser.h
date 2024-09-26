#ifndef SYLAR_TEST_HTTP_PARSER_H
#define SYLAR_TEST_HTTP_PARSER_H

#include "HttpParser.h"
#include "Log.h"
#include <iostream>
using std::cout;
using std::endl;
using namespace sylar;

namespace Test
{

const char test_request_data[] = "POST / HTTP/1.1\r\n"
                                 "Host: www.sylar.top\r\n"
                                 "Content-Length: 10\r\n\r\n"
                                 "1234567890";

void test_httpparser_request() {
    sylar::http::HttpRequestParser parser;
    std::string tmp = test_request_data;
    size_t s = parser.execute(&tmp[0], tmp.size());
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
        << "execute rt=" << s
        << "has_error=" << parser.hasError()
        << " is_finished=" << parser.isFinished()
        << " total=" << tmp.size()
        << " content_length=" << parser.getContentLength();
    tmp.resize(tmp.size() - s);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << parser.getData()->toString();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << tmp;
}

const char test_response_data[] = "HTTP/1.1 200 OK\r\n"
                                  "Date: Tue, 04 Jun 2019 15:43:56 GMT\r\n"
                                  "Server: Apache\r\n"
                                  "Last-Modified: Tue, 12 Jan 2010 13:48:00 GMT\r\n"
                                  "ETag: \"51-47cf7e6ee8400\"\r\n"
                                  "Accept-Ranges: bytes\r\n"
                                  "Content-Length: 81\r\n"
                                  "Cache-Control: max-age=86400\r\n"
                                  "Expires: Wed, 05 Jun 2019 15:43:56 GMT\r\n"
                                  "Connection: Close\r\n"
                                  "Content-Type: text/html\r\n\r\n"
                                  "<html>\r\n"
                                  "<meta http-equiv=\"refresh\" content=\"0;url=http://www.baidu.com/\">\r\n"
                                  "</html>\r\n";

void test_httpparser_response() {
    sylar::http::HttpResponseParser parser;
    std::string tmp = test_response_data;
    size_t s = parser.execute(&tmp[0], tmp.size(), true);
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
        << "execute rt=" << s
        << " has_error=" << parser.hasError()
        << " is_finished=" << parser.isFinished()
        << " total=" << tmp.size()
        << " content_length=" << parser.getContentLength()
        << " tmp[s]=" << tmp[s];

    tmp.resize(tmp.size() - s);

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << parser.getData()->toString();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << tmp;
}


void test_httpparser() {
	cout << "----------------------- test HttpParse -----------------------------" << endl;
    test_httpparser_request();
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "---------------------------";
    test_httpparser_response();
	cout << "----------------------- test over -----------------------------" << endl;
}

}; /* Test */

#endif /* SYLAR_TEST_HTTP_PARSER_H */
