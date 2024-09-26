//*****************************************************************************
//
//
//   此头文件封装 HttpSession
//   HttpSession接收请求报文，发送响应报文
//  
//
//*****************************************************************************


#ifndef SYLAR_HTTP_SESSION_H
#define SYLAR_HTTP_SESSION_H

#include "Stream.h"
#include "Http.h"
#include <memory>

namespace sylar::http
{

//****************************************************************************
// 前置声明
//****************************************************************************

class HttpSession;
using HttpSession_ptr = std::shared_ptr<HttpSession>;

//****************************************************************************
// HttpSession::封装
//****************************************************************************

class HttpSession : public sylar::SocketStream {
public:
    /*!
     * @brief 构造函数
     * @param sock Socket类型
     * @param owner 是否托管
     */
    HttpSession(sylar::Socket_ptr sock, bool owner = true);

    /*!
     * @brief 接收HTTP请求
     */
    HttpRequest_ptr recvRequest();

    /*!
     * @brief 发送HTTP响应
     * @param rsp HTTP响应
     * @return 
     *      @retval > 0 发送成功
     *      @retval = 0 对方关闭
     *      @retval < 0 Socket异常
     */
    int sendResponse(HttpResponse_ptr rsp);
};

}; /* sylar::http */

#endif /* SYLAR_HTTP_SESSION_H*/
