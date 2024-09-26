#ifndef SYLAR_TEST_HTTP_SERVER_H
#define SYLAR_TEST_HTTP_SERVER_H

#include "HttpServer.h"
#include "IOManager.h"
#include <iostream>

using std::cout;
using std::endl;

namespace Test
{

static sylar::Logger_ptr g_logger = sylar::SYLAR_LOG_ROOT();

#define XX(...) #__VA_ARGS__

sylar::IOManager_ptr worker;

void test_httpserver_run() {
    g_logger->setLevel(sylar::LogLevel::INFO);
    sylar::http::HttpServer_ptr server(new sylar::http::HttpServer(true, worker.get(), sylar::IOManager::GetThis()));
    // sylar::http::HttpServer_ptr server(new sylar::http::HttpServer);
    sylar::Address_ptr addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while (!server->bind(addr)) {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/sylar/xx", [](sylar::http::HttpRequest_ptr req, 
                                   sylar::http::HttpResponse_ptr rsp, 
                                   sylar::http::HttpSession_ptr session) {
        rsp->setBody(req->toString());
        return 0;
    });

    sd->addGlobServlet("/sylar/*", [](sylar::http::HttpRequest_ptr req, 
                                      sylar::http::HttpResponse_ptr rsp, 
                                      sylar::http::HttpSession_ptr session) {
        rsp->setBody("Glob:\r\n" + req->toString());
        return 0;
    });

    sd->addGlobServlet("/sylarx/*", [](sylar::http::HttpRequest_ptr req, 
                                       sylar::http::HttpResponse_ptr rsp, 
                                       sylar::http::HttpSession_ptr session) {
        rsp->setBody(XX(<html>
                            <head> 
                                <title>
                                    404 Not Found
                                </title>
                            </head>
                            <body>
                                <center> 
                                    <h1>
                                        404 Not Found
                                    </h1></center>
                                    <hr>
                                        nginx / 1.16.0 
                                    </hr>
                                </center>
                            </body>
                        </html>
                        <!--a padding to disable MSIE and Chrome friendly error page-->
                        <!--a padding to disable MSIE and Chrome friendly error page-->
                        <!--a padding to disable MSIE and Chrome friendly error page-->
                        <!--a padding to disable MSIE and Chrome friendly error page-->
                        <!--a padding to disable MSIE and Chrome friendly error page-->
                        <!--a padding to disable MSIE and Chrome friendly error page-->
        ));
        return 0;
    });

    server->start();
}

void test_httpserver() {
	cout << "---------------------------- test HttpServer -------------------------" << endl;
    sylar::IOManager iom(1, true, "main");
    worker.reset(new sylar::IOManager(3, false, "worker"));
    iom.schedule(test_httpserver_run);
	cout << "---------------------------- test over -------------------------" << endl;
}

}; /* Test */

#endif /* SYLAR_TEST_HTTP_SERVER_H */
