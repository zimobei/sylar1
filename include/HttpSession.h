//*****************************************************************************
//
//
//   ��ͷ�ļ���װ HttpSession
//   HttpSession���������ģ�������Ӧ����
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
// ǰ������
//****************************************************************************

class HttpSession;
using HttpSession_ptr = std::shared_ptr<HttpSession>;

//****************************************************************************
// HttpSession::��װ
//****************************************************************************

class HttpSession : public sylar::SocketStream {
public:
    /*!
     * @brief ���캯��
     * @param sock Socket����
     * @param owner �Ƿ��й�
     */
    HttpSession(sylar::Socket_ptr sock, bool owner = true);

    /*!
     * @brief ����HTTP����
     */
    HttpRequest_ptr recvRequest();

    /*!
     * @brief ����HTTP��Ӧ
     * @param rsp HTTP��Ӧ
     * @return 
     *      @retval > 0 ���ͳɹ�
     *      @retval = 0 �Է��ر�
     *      @retval < 0 Socket�쳣
     */
    int sendResponse(HttpResponse_ptr rsp);
};

}; /* sylar::http */

#endif /* SYLAR_HTTP_SESSION_H*/
