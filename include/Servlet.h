//*****************************************************************************
//
//
//   此头文件封装 Servlet
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
// 前置声明
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
// Servlet封装
//****************************************************************************

class Servlet {
protected:
    std::string m_name; // 名称
public:
    /*!
     * @brief 构造函数
     * @param name 名称
     */
    Servlet(const std::string& name);

    /*!
     * @brief 析构函数
     */
    virtual ~Servlet();

    /*!
     * @brief 处理请求
     * @param request HTTP请求
     * @param response HTTP响应
     * @param session HTTP连接
     * @return 是否处理成功
     */
    virtual int32_t handle(HttpRequest_ptr request, 
                           HttpResponse_ptr response, 
                           HttpSession_ptr session) = 0;

    /*!
     * @brief 返回Servlet名称
     */
    const std::string& getName() const;
};

//****************************************************************************
// 函数式Servlet
//****************************************************************************

class FunctionServlet : public Servlet {
public:
    // 函数回调类型定义
    using callback = std::function<int32_t(HttpRequest_ptr request, HttpResponse_ptr response, HttpSession_ptr session)>;
private:
    callback m_cb;  // 回调函数
public:
    /*!
     * @brief 构造函数
     * @param cb 回调函数
     */
    FunctionServlet(callback cb);

    virtual int32_t handle(HttpRequest_ptr request, 
                           HttpResponse_ptr response, 
                           HttpSession_ptr session) override;
};

//****************************************************************************
// 默认返回404
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
// Servlet 分发器/管理器
//****************************************************************************

class ServletDispatch : public Servlet {
public:
    using RWMutexType = RWMutex;
private:
    RWMutexType m_mutex;                                                // 读写互斥量
    std::unordered_map<std::string, Servlet_ptr> m_datas;               // 精准匹配servlet MAP [ uri(/sylar/xxx) -> servlet ]
    std::vector<std::pair<std::string, Servlet_ptr>> m_globs;           // 模糊匹配servlet 数组 [ uri(/sylar/*) -> servlet ]
    Servlet_ptr m_default;                                              // 默认servlet，所有路径都没匹配到时使用
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
// 模板类或函数实现
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
