//*****************************************************************************
//
//
//   HTTP定义结构体封装
//  
//
//*****************************************************************************

#ifndef SYLAR_HTTP_H
#define SYLAR_HTTP_H

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>

namespace sylar::http
{

//****************************************************************************
// 前置声明
//****************************************************************************

enum class HttpMethod;

enum class HttpStatus;

class HttpRequest;
using HttpRequest_ptr = std::shared_ptr<HttpRequest>;

class HttpResponse;
using HttpResponse_ptr = std::shared_ptr<HttpResponse>;

//****************************************************************************
// HTTP方法枚举
//****************************************************************************

#define HTTP_METHOD_MAP(XX)         \
  XX(0,  DELETE,      DELETE)       \
  XX(1,  GET,         GET)          \
  XX(2,  HEAD,        HEAD)         \
  XX(3,  POST,        POST)         \
  XX(4,  PUT,         PUT)          \
  /* pathological */                \
  XX(5,  CONNECT,     CONNECT)      \
  XX(6,  OPTIONS,     OPTIONS)      \
  XX(7,  TRACE,       TRACE)        \
  /* WebDAV */                      \
  XX(8,  COPY,        COPY)         \
  XX(9,  LOCK,        LOCK)         \
  XX(10, MKCOL,       MKCOL)        \
  XX(11, MOVE,        MOVE)         \
  XX(12, PROPFIND,    PROPFIND)     \
  XX(13, PROPPATCH,   PROPPATCH)    \
  XX(14, SEARCH,      SEARCH)       \
  XX(15, UNLOCK,      UNLOCK)       \
  XX(16, BIND,        BIND)         \
  XX(17, REBIND,      REBIND)       \
  XX(18, UNBIND,      UNBIND)       \
  XX(19, ACL,         ACL)          \
  /* subversion */                  \
  XX(20, REPORT,      REPORT)       \
  XX(21, MKACTIVITY,  MKACTIVITY)   \
  XX(22, CHECKOUT,    CHECKOUT)     \
  XX(23, MERGE,       MERGE)        \
  /* upnp */                        \
  XX(24, MSEARCH,     M-SEARCH)     \
  XX(25, NOTIFY,      NOTIFY)       \
  XX(26, SUBSCRIBE,   SUBSCRIBE)    \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                    \
  XX(28, PATCH,       PATCH)        \
  XX(29, PURGE,       PURGE)        \
  /* CalDAV */                      \
  XX(30, MKCALENDAR,  MKCALENDAR)   \
  /* RFC-2068, section 19.6.1.2 */  \
  XX(31, LINK,        LINK)         \
  XX(32, UNLINK,      UNLINK)       \
  /* icecast */                     \
  XX(33, SOURCE,      SOURCE)       \

enum class HttpMethod {
#define XX(num, name, string) name = num,
    HTTP_METHOD_MAP(XX)
#undef XX
    INVALID_METHOD
};

//****************************************************************************
// HTTP状态枚举
//****************************************************************************

#define HTTP_STATUS_MAP(XX)                                                 \
  XX(100, CONTINUE,                        Continue)                        \
  XX(101, SWITCHING_PROTOCOLS,             Switching Protocols)             \
  XX(102, PROCESSING,                      Processing)                      \
  XX(200, OK,                              OK)                              \
  XX(201, CREATED,                         Created)                         \
  XX(202, ACCEPTED,                        Accepted)                        \
  XX(203, NON_AUTHORITATIVE_INFORMATION,   Non-Authoritative Information)   \
  XX(204, NO_CONTENT,                      No Content)                      \
  XX(205, RESET_CONTENT,                   Reset Content)                   \
  XX(206, PARTIAL_CONTENT,                 Partial Content)                 \
  XX(207, MULTI_STATUS,                    Multi-Status)                    \
  XX(208, ALREADY_REPORTED,                Already Reported)                \
  XX(226, IM_USED,                         IM Used)                         \
  XX(300, MULTIPLE_CHOICES,                Multiple Choices)                \
  XX(301, MOVED_PERMANENTLY,               Moved Permanently)               \
  XX(302, FOUND,                           Found)                           \
  XX(303, SEE_OTHER,                       See Other)                       \
  XX(304, NOT_MODIFIED,                    Not Modified)                    \
  XX(305, USE_PROXY,                       Use Proxy)                       \
  XX(307, TEMPORARY_REDIRECT,              Temporary Redirect)              \
  XX(308, PERMANENT_REDIRECT,              Permanent Redirect)              \
  XX(400, BAD_REQUEST,                     Bad Request)                     \
  XX(401, UNAUTHORIZED,                    Unauthorized)                    \
  XX(402, PAYMENT_REQUIRED,                Payment Required)                \
  XX(403, FORBIDDEN,                       Forbidden)                       \
  XX(404, NOT_FOUND,                       Not Found)                       \
  XX(405, METHOD_NOT_ALLOWED,              Method Not Allowed)              \
  XX(406, NOT_ACCEPTABLE,                  Not Acceptable)                  \
  XX(407, PROXY_AUTHENTICATION_REQUIRED,   Proxy Authentication Required)   \
  XX(408, REQUEST_TIMEOUT,                 Request Timeout)                 \
  XX(409, CONFLICT,                        Conflict)                        \
  XX(410, GONE,                            Gone)                            \
  XX(411, LENGTH_REQUIRED,                 Length Required)                 \
  XX(412, PRECONDITION_FAILED,             Precondition Failed)             \
  XX(413, PAYLOAD_TOO_LARGE,               Payload Too Large)               \
  XX(414, URI_TOO_LONG,                    URI Too Long)                    \
  XX(415, UNSUPPORTED_MEDIA_TYPE,          Unsupported Media Type)          \
  XX(416, RANGE_NOT_SATISFIABLE,           Range Not Satisfiable)           \
  XX(417, EXPECTATION_FAILED,              Expectation Failed)              \
  XX(421, MISDIRECTED_REQUEST,             Misdirected Request)             \
  XX(422, UNPROCESSABLE_ENTITY,            Unprocessable Entity)            \
  XX(423, LOCKED,                          Locked)                          \
  XX(424, FAILED_DEPENDENCY,               Failed Dependency)               \
  XX(426, UPGRADE_REQUIRED,                Upgrade Required)                \
  XX(428, PRECONDITION_REQUIRED,           Precondition Required)           \
  XX(429, TOO_MANY_REQUESTS,               Too Many Requests)               \
  XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
  XX(451, UNAVAILABLE_FOR_LEGAL_REASONS,   Unavailable For Legal Reasons)   \
  XX(500, INTERNAL_SERVER_ERROR,           Internal Server Error)           \
  XX(501, NOT_IMPLEMENTED,                 Not Implemented)                 \
  XX(502, BAD_GATEWAY,                     Bad Gateway)                     \
  XX(503, SERVICE_UNAVAILABLE,             Service Unavailable)             \
  XX(504, GATEWAY_TIMEOUT,                 Gateway Timeout)                 \
  XX(505, HTTP_VERSION_NOT_SUPPORTED,      HTTP Version Not Supported)      \
  XX(506, VARIANT_ALSO_NEGOTIATES,         Variant Also Negotiates)         \
  XX(507, INSUFFICIENT_STORAGE,            Insufficient Storage)            \
  XX(508, LOOP_DETECTED,                   Loop Detected)                   \
  XX(510, NOT_EXTENDED,                    Not Extended)                    \
  XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required) \

enum class HttpStatus {
#define XX(code, name, desc) name = code,
    HTTP_STATUS_MAP(XX)
#undef XX
};

//****************************************************************************
// HTTP方法与状态相关函数
//****************************************************************************

/*!
 * @brief 将字符串方法名转成HTTP方法枚举
 * @param m HTTP方法
 * @return HTTP方法枚举
 */
HttpMethod StringToHttpMethod(const std::string& m);

/*!
 * @brief 将字符串指针转换成HTTP方法枚举
 * @param m 字符串方法枚举
 * @return HTTP方法枚举
 */
HttpMethod CharsToHttpMethod(const char* m);

/*!
 * @brief 将HTTP方法枚举转换成字符串
 * @param m HTTP方法枚举
 * @return 字符串
 */
const char* HttpMethodToString(const HttpMethod& m);

/*!
 * @brief 将HTTP状态枚举转换成字符串
 * @param s HTTP状态枚举
 * @return 字符串
 */
const char* HttpStatusToString(const HttpStatus& s);

/*!
 * @brief 获取Map中的key值,并转成对应类型,返回是否成功
 * @param m Map数据结构
 * @param key 关键字
 * @param val 保存转换后的值
 * @param def 默认值
 * @return 
 *      @retval true 转换成功, val 为对应的值
 *      @retval false 不存在或者转换失败 val = def
 */
template<class MapType, class T>
bool checkGetAs(const MapType& m, const std::string& key, T& val, const T& def = T());

/*!
 * @brief 获取Map中的key值,并转成对应类型
 * @param m Map数据结构
 * @param key 关键字
 * @param def 默认值
 * @return 如果存在且转换成功返回对应的值,否则返回默认值
 */
template<class MapType, class T>
T getAs(const MapType& m, const std::string& key, const T& def = T());

/*!
 * @brief 忽略大小写比较仿函数
 */
struct CaseInsensitiveLess {
    /**
     * @brief 忽略大小写比较字符串
     */
    bool operator()(const std::string& lhs, const std::string& rhs) const;
};

//****************************************************************************
// HTTP请求结构
//****************************************************************************

class HttpRequest {
public:
    using MapType = std::map<std::string, std::string, CaseInsensitiveLess>;
private:
    HttpMethod __method;        // HTTP方法
    uint8_t __version;          // HTTP版本
    bool __close;               // 是否自动关闭
    bool __websocket;           // 是否为websocket
    uint8_t __parserParamFlag;
    std::string __path;         // 请求路径
    std::string __query;        // 请求参数
    std::string __fragment;     // 请求fragment
    std::string __body;         // 请求消息体
    MapType __headers;          // 请求头部MAP
    MapType __params;           // 请求参数MAP
    MapType __cookies;          // 请求Cookie MAP
public:
    HttpRequest(uint8_t version = 0x11, bool close = true);

    HttpResponse_ptr createResponse();

    HttpMethod getMethod() const;

    uint8_t getVersion() const;

    const std::string& getPath() const;

    const std::string& getQuery() const;

    const std::string& getBody() const;

    const MapType& getHeaders() const;

    const MapType& getParams() const;

    const MapType& getCookies() const;

    void setMethod(HttpMethod v);

    void setVersion(uint8_t v);

    void setPath(const std::string& v);

    void setQuery(const std::string& v);

    void setFragment(const std::string& v);

    void setBody(const std::string& v);

    bool isClose() const;

    void setClose(bool v);

    bool isWebsocket() const;

    void setWebsocket(bool v);

    void setHeaders(const MapType& v);

    void setParams(const MapType& v);

    void setCookies(const MapType& v);

    std::string getHeader(const std::string& key, const std::string& def = "") const;

    std::string getParam(const std::string& key, const std::string& def = "");

    std::string getCookie(const std::string& key, const std::string& def = "");

    void setHeader(const std::string& key, const std::string& val);

    void setParam(const std::string& key, const std::string& val);

    void setCookie(const std::string& key, const std::string& val);

    void delHeader(const std::string& key);

    void delParam(const std::string& key);

    void delCookie(const std::string& key);

    bool hasHeader(const std::string& key, std::string* val = nullptr);

    bool hasParam(const std::string& key, std::string* val = nullptr);

    bool hasCookie(const std::string& key, std::string* val = nullptr);

    template<class T>
    bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T());

    template<class T>
    T getHeaderAs(const std::string& key, const T& def = T());

    template<class T>
    bool checkGetParamAs(const std::string& key, T& val, const T& def = T());

    template<class T>
    T getParamAs(const std::string& key, const T& def = T());

    template<class T>
    bool checkGetCookieAs(const std::string& key, T& val, const T& def = T());

    template<class T>
    T getCookieAs(const std::string& key, const T& def = T());

    std::ostream& dump(std::ostream& os) const;

    std::string toString() const;

    void init();
    void initParam();
    void initQueryParam();
    void initBodyParam();
    void initCookies();
};

//****************************************************************************
// HTTP响应结构
//****************************************************************************

class HttpResponse {
public:
    using MapType = std::map<std::string, std::string, CaseInsensitiveLess>;
private:
    HttpStatus __status;                // 响应状态
    uint8_t __version;                  // 版本
    bool __close;                       // 是否自动关闭
    bool __websocket;                   // 是否为websocket 
    std::string __body;                 // 响应消息体
    std::string __reason;               // 响应原因
    MapType __headers;                  // 响应头部MAP
    std::vector<std::string> __cookies;
public:
    HttpResponse(uint8_t version = 0x11, bool close = true);

    HttpStatus getStatus() const;

    uint8_t getVersion() const;

    const std::string& getBody() const;

    const std::string& getReason() const;

    const MapType& getHeaders() const;

    void setStatus(HttpStatus v);

    void setVersion(uint8_t v);

    void setBody(const std::string& v);

    void setReason(const std::string& v);

    void setHeaders(const MapType& v);

    bool isClose() const;

    void setClose(bool v);

    bool isWebsocket() const;

    void setWebsocket(bool v);

    std::string getHeader(const std::string& key, const std::string& def = "") const;

    void setHeader(const std::string& key, const std::string& val);

    void delHeader(const std::string& key);

    template<class T>
    bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T());

    template<class T>
    T getHeaderAs(const std::string& key, const T& def = T());

    std::ostream& dump(std::ostream& os) const;

    std::string toString() const;

    void setRedirect(const std::string& uri);
    void setCookie(const std::string& key, const std::string& val,
                   time_t expired = 0, const std::string& path = "",
                   const std::string& domain = "", bool secure = false);
};

//****************************************************************************
// 流式输出
//****************************************************************************

std::ostream& operator<<(std::ostream& os, const HttpRequest& req);

std::ostream& operator<<(std::ostream& os, const HttpResponse& rsp);

//****************************************************************************
// 模板类或函数实现
//****************************************************************************

template<class MapType, class T>
bool checkGetAs(const MapType& m, const std::string& key, T& val, const T& def) {
    auto it = m.find(key);
    if (it == m.end()) {
        val = def;
        return false;
    }
    try {
        val = boost::lexical_cast<T>(it->second);
        return true;
    }
    catch (...) {
        val = def;
    }
    return false;
}

template<class MapType, class T>
T getAs(const MapType& m, const std::string& key, const T& def) {
    auto it = m.find(key);
    if (it == m.end()) {
        return def;
    }
    try {
        return boost::lexical_cast<T>(it->second);
    }
    catch (...) {
    }
    return def;
}

template<class T>
bool HttpRequest::checkGetHeaderAs(const std::string& key, T& val, const T& def) {
    return checkGetAs(__headers, key, val, def);
}

template<class T>
T HttpRequest::getHeaderAs(const std::string& key, const T& def) {
    return getAs(__headers, key, def);
}

template<class T>
bool HttpRequest::checkGetParamAs(const std::string& key, T& val, const T& def) {
    initQueryParam();
    initBodyParam();
    return checkGetAs(__params, key, val, def);
}

template<class T>
T HttpRequest::getParamAs(const std::string& key, const T& def) {
    initQueryParam();
    initBodyParam();
    return getAs(__params, key, def);
}

template<class T>
bool HttpRequest::checkGetCookieAs(const std::string& key, T& val, const T& def) {
    initCookies();
    return checkGetAs(__cookies, key, val, def);
}

template<class T>
T HttpRequest::getCookieAs(const std::string& key, const T& def) {
    initCookies();
    return getAs(__cookies, key, def);
}

template<class T>
bool HttpResponse::checkGetHeaderAs(const std::string& key, T& val, const T& def) {
    return checkGetAs(__headers, key, val, def);
}

template<class T>
T HttpResponse::getHeaderAs(const std::string& key, const T& def) {
    return getAs(__headers, key, def);
}

}; /* sylar::http */

#endif /* SYLAR_HTTP_H */
