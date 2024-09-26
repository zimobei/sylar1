#include "HttpServer.h"
#include "Log.h"

namespace sylar::http
{

//****************************************************************************
// HttpServer
//****************************************************************************

void HttpServer::handleClient(Socket_ptr client) {
    SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << "handleClient " << *client;
    HttpSession_ptr session(new HttpSession(client));
    do {
        auto req = session->recvRequest();
        if (!req) {
            SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) 
                << "recv http request fail, errno=" << errno 
                << " errstr=" << strerror(errno)
                << " cliet:" << *client 
                << " keep_alive=" << m_isKeepalive;
            break;
        }

        HttpResponse_ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));
        rsp->setHeader("Server", getName());
        m_dispatch->handle(req, rsp, session);
        session->sendResponse(rsp);

        if (!m_isKeepalive || req->isClose()) {
            break;
        }
    } while (true);
    session->close();
}

HttpServer::HttpServer(bool keepalive,
					   sylar::IOManager* worker,
					   sylar::IOManager* io_worker,
					   sylar::IOManager* accept_worker) 
	: TcpServer(worker, io_worker, accept_worker)
	, m_isKeepalive(keepalive) {
	m_dispatch.reset(new ServletDispatch);
}

ServletDispatch_ptr HttpServer::getServletDispatch() const { 
	return m_dispatch; 
}

void HttpServer::setServletDispatch(ServletDispatch_ptr v) { 
	m_dispatch = v; 
}

void HttpServer::setName(const std::string& v) {
	TcpServer::setName(v);
	m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
}

}; /* sylar::http */