//*****************************************************************************
//
//
//   ��ͷ�ļ���װ Servlet
//  
//
//*****************************************************************************


#ifndef SYLAR_SERVLET_H
#define SYLAR_SERVLET_H

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include "Http.h"
#include "HttpSession.h"
#include "Mutex.h"
#include "Thread.h"
#include "Util.h"

namespace sylar::http
{

//****************************************************************************
// ǰ������
//****************************************************************************
class Servlet;
using Servlet_ptr = std::shared_ptr<Servlet>;

class FunctionServlet;
using FunctionServlet_ptr = std::shared_ptr<FunctionServlet>;

class NotFoundServlet;
using NotFoundServlet_ptr = std::shared_ptr<NotFoundServlet>;

class ServletDispatch;
using ServletDispatch_ptr = std::shared_ptr<ServletDispatch>;

//****************************************************************************
// Servlet��װ
//****************************************************************************

class Servlet {
protected:
    std::string m_name; // ����
public:
    /*!
     * @brief ���캯��
     * @param name ����
     */
    Servlet(const std::string& name);

    /*!
     * @brief ��������
     */
    virtual ~Servlet();

    /*!
     * @brief ��������
     * @param request HTTP����
     * @param response HTTP��Ӧ
     * @param session HTTP����
     * @return �Ƿ���ɹ�
     */
    virtual int32_t handle(HttpRequest_ptr request, 
                           HttpResponse_ptr response, 
                           HttpSession_ptr session) = 0;

    /*!
     * @brief ����Servlet����
     */
    const std::string& getName() const;
};

//****************************************************************************
// ����ʽServlet
//****************************************************************************

class FunctionServlet : public Servlet {
public:
    // �����ص����Ͷ���
    using callback = std::function<int32_t(HttpRequest_ptr request, HttpResponse_ptr response, HttpSession_ptr session)>;
private:
    callback m_cb;  // �ص�����
public:
    /*!
     * @brief ���캯��
     * @param cb �ص�����
     */
    FunctionServlet(callback cb);

    virtual int32_t handle(HttpRequest_ptr request, 
                           HttpResponse_ptr response, 
                           HttpSession_ptr session) override;
};

//****************************************************************************
// Ĭ�Ϸ���404
//****************************************************************************

class NotFoundServlet : public Servlet {
private:
    std::string m_name;
    std::string m_content;
public:
    NotFoundServlet(const std::string& name);
    virtual int32_t handle(HttpRequest_ptr request,
                           HttpResponse_ptr response,
                           HttpSession_ptr session) override;
};

//****************************************************************************
// Servlet �ַ���/������
//****************************************************************************

class ServletDispatch : public Servlet {
public:
    using RWMutexType = RWMutex;
private:
    RWMutexType m_mutex;                                                // ��д������
    std::unordered_map<std::string, Servlet_ptr> m_datas;               // ��׼ƥ��servlet MAP [ uri(/sylar/xxx) -> servlet ]
    std::vector<std::pair<std::string, Servlet_ptr>> m_globs;           // ģ��ƥ��servlet ���� [ uri(/sylar/*) -> servlet ]
    Servlet_ptr m_default;                                              // Ĭ��servlet������·����ûƥ�䵽ʱʹ��
public:
    ServletDispatch();
    virtual int32_t handle(HttpRequest_ptr request, HttpResponse_ptr response, HttpSession_ptr session) override;

    void addServlet(const std::string& uri, Servlet_ptr slt);

    void addServlet(const std::string& uri, FunctionServlet::callback cb);

    void addGlobServlet(const std::string& uri, Servlet_ptr slt);

    void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);

    //void addServletCreator(const std::string& uri, IServletCreator_ptr creator);
    //void addGlobServletCreator(const std::string& uri, IServletCreator_ptr creator);

    //template<class T>
    //void addServletCreator(const std::string& uri);

    //template<class T>
    //void addGlobServletCreator(const std::string& uri);

    void delServlet(const std::string& uri);

    void delGlobServlet(const std::string& uri);

    Servlet_ptr getDefault() const;

    void setDefault(Servlet_ptr v);

    Servlet_ptr getServlet(const std::string& uri);

    Servlet_ptr getGlobServlet(const std::string& uri);

    Servlet_ptr getMatchedServlet(const std::string& uri);

    //void listAllServletCreator(std::map<std::string, IServletCreator_ptr>& infos);
    //void listAllGlobServletCreator(std::map<std::string, IServletCreator_ptr>& infos);
};

//****************************************************************************
// ģ�������ʵ��
//****************************************************************************

//template<class T>
//void ServletDispatch::addServletCreator(const std::string& uri) {
//    addServletCreator(uri, std::make_shared<ServletCreator<T>>());
//}
//
//template<class T>
//void ServletDispatch::addGlobServletCreator(const std::string& uri) {
//    addGlobServletCreator(uri, std::make_shared<ServletCreator<T>>());
//}

}; /* sylar::http */

#endif /* SYLAR_SERVLET_H */
