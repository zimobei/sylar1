//*****************************************************************************
//
//
//   HTTP服务器封装
//  
//
//*****************************************************************************


#ifndef SYLAR_HTTP_SERVER_H
#define SYLAR_HTTP_SERVER_H

#include "TcpServer.h"
#include "HttpSession.h"
#include "Servlet.h"
#include "IOManager.h"
#include <memory>

namespace sylar::http
{

//****************************************************************************
// 前置声明
//****************************************************************************

class HttpServer;
using HttpServer_ptr = std::shared_ptr<HttpServer>;

//****************************************************************************
// HTTP服务器类
//****************************************************************************

class HttpServer : public sylar::TcpServer {
private: 
    bool m_isKeepalive;                 // 是否支持长连接
    ServletDispatch_ptr m_dispatch;    // Servlet分发器
protected:
    virtual void handleClient(Socket_ptr client) override;
public:
    HttpServer(bool keepalive = false, 
               sylar::IOManager* worker = sylar::IOManager::GetThis(), 
               sylar::IOManager* io_worker = sylar::IOManager::GetThis(), 
               sylar::IOManager* accept_worker = sylar::IOManager::GetThis());

    ServletDispatch_ptr getServletDispatch() const;

    void setServletDispatch(ServletDispatch_ptr v);

    virtual void setName(const std::string& v) override;
};

}; /* sylar::http */

#endif /* SYLAR_HTTP_SERVER_H */
