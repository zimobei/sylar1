//*****************************************************************************
//
//
//   此头文件封装 URI
// 
//   URI 格式为:
// 
//   foo://user@sylar.com:8042/over/there?name=ferret#nose
//      \_/ \______________/ \_________/ \_________/ \__/
//       |          |             |           |        |
//     scheme     authority       path        query   fragment
// 
//*****************************************************************************

#ifndef SYLAR_URI_H
#define SYLAR_URI_H

#include <memory>
#include <string>
#include <stdint.h>
#include "Address.h"

namespace sylar
{

//****************************************************************************
// 前置声明
//****************************************************************************

class Uri;
using Uri_ptr = std::shared_ptr<Uri>;

//****************************************************************************
// URI 类
//****************************************************************************

class Uri {
private:
    std::string m_scheme;           // schema
    std::string m_userinfo;         // 用户信息
    std::string m_host;             // host
    std::string m_path;             // 路径
    std::string m_query;            // 查询参数
    std::string m_fragment;         // fragment
    int32_t m_port;                 // 端口
private:
    bool isDefaultPort() const;
public:
    static Uri_ptr Create(const std::string& uri);

    Uri();

    const std::string& getScheme() const { return m_scheme; }

    const std::string& getUserinfo() const { return m_userinfo; }

    const std::string& getHost() const { return m_host; }

    const std::string& getPath() const;

    const std::string& getQuery() const { return m_query; }

    const std::string& getFragment() const { return m_fragment; }

    int32_t getPort() const;

    void setScheme(const std::string& v) { m_scheme = v; }

    void setUserinfo(const std::string& v) { m_userinfo = v; }

    void setHost(const std::string& v) { m_host = v; }

    void setPath(const std::string& v) { m_path = v; }

    void setQuery(const std::string& v) { m_query = v; }

    void setFragment(const std::string& v) { m_fragment = v; }

    void setPort(int32_t v) { m_port = v; }

    std::ostream& dump(std::ostream& os) const;

    std::string toString() const;

    Address_ptr createAddress() const;
};

}; /* sylar */

#endif /* SYLAR_URI_H */
