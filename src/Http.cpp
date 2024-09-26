#include "Http.h"
#include "Util.h"

namespace sylar::http   
{

//****************************************************************************
// HTTP方法与状态相关函数
//****************************************************************************

HttpMethod StringToHttpMethod(const std::string& m) {
#define XX(num, name, string) \
    if(strcmp(#string, m.c_str()) == 0) { \
        return HttpMethod::name; \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return HttpMethod::INVALID_METHOD;
}

HttpMethod CharsToHttpMethod(const char* m) {
#define XX(num, name, string) \
    if(strncmp(#string, m, strlen(#string)) == 0) { \
        return HttpMethod::name; \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return HttpMethod::INVALID_METHOD;
}

static const char* s_method_string[] = {
#define XX(num, name, string) #string,
    HTTP_METHOD_MAP(XX)
#undef XX
};

const char* HttpMethodToString(const HttpMethod& m) {
    uint32_t idx = (uint32_t)m;
    if (idx >= (sizeof(s_method_string) / sizeof(s_method_string[0]))) {
        return "<unknown>";
    }
    return s_method_string[idx];
}

const char* HttpStatusToString(const HttpStatus& s) {
    switch (s) {
    #define XX(code, name, msg) \
        case HttpStatus::name: \
            return #msg;
        HTTP_STATUS_MAP(XX);
    #undef XX
        default:
            return "<unknown>";
    }
}

bool CaseInsensitiveLess::operator()(const std::string& lhs,
                                     const std::string& rhs) const {
    return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
}

//****************************************************************************
// HttpRequest
//****************************************************************************

HttpRequest::HttpRequest(uint8_t version, bool close) 
    :__method(HttpMethod::GET)
    , __version(version)
    , __close(close)
    , __websocket(false)
    , __parserParamFlag(0)
    , __path("/") {}

HttpResponse_ptr HttpRequest::createResponse() {
    HttpResponse_ptr rsp(new HttpResponse(getVersion(), isClose()));
    return rsp;
}

HttpMethod HttpRequest::getMethod() const {
    return __method;
}

uint8_t HttpRequest::getVersion() const {
    return __version;
}

const std::string& HttpRequest::getPath() const {
    return __path;
}

const std::string& HttpRequest::getQuery() const {
    return __query;
}

const std::string& HttpRequest::getBody() const {
    return __body;
}

const HttpRequest::MapType& HttpRequest::getHeaders() const {
    return __headers;
}

const HttpRequest::MapType& HttpRequest::getParams() const {
    return __params;
}

const HttpRequest::MapType& HttpRequest::getCookies() const {
    return __cookies;
}

void HttpRequest::setMethod(HttpMethod v) {
    __method = v;
}

void HttpRequest::setVersion(uint8_t v) {
    __version = v;
}

void HttpRequest::setPath(const std::string& v) {
    __path = v;
}

void HttpRequest::setQuery(const std::string& v) {
    __query = v;
}

void HttpRequest::setFragment(const std::string& v) {
    __fragment = v;
}

void HttpRequest::setBody(const std::string& v) {
    __body = v;
}

bool HttpRequest::isClose() const {
    return __close;
}

void HttpRequest::setClose(bool v) {
    __close = v;
}

bool HttpRequest::isWebsocket() const {
    return __websocket;
}

void HttpRequest::setWebsocket(bool v) {
    __websocket = v;
}

void HttpRequest::setHeaders(const HttpRequest::MapType& v) {
    __headers = v;
}

void HttpRequest::setParams(const HttpRequest::MapType& v) {
    __params = v;
}

void HttpRequest::setCookies(const HttpRequest::MapType& v) {
    __cookies = v;
}

std::string HttpRequest::getHeader(const std::string& key, const std::string& def) const {
    auto it = __headers.find(key);
    return it == __headers.end() ? def : it->second;
}

std::string HttpRequest::getParam(const std::string& key, const std::string& def) {
    initQueryParam();
    initBodyParam();
    auto it = __params.find(key);
    return it == __params.end() ? def : it->second;
}

std::string HttpRequest::getCookie(const std::string& key, const std::string& def) {
    initCookies();
    auto it = __cookies.find(key);
    return it == __cookies.end() ? def : it->second;
}

void HttpRequest::setHeader(const std::string& key, const std::string& val) {
    __headers[key] = val;
}

void HttpRequest::setParam(const std::string& key, const std::string& val) {
    __params[key] = val;
}

void HttpRequest::setCookie(const std::string& key, const std::string& val) {
    __cookies[key] = val;
}

void HttpRequest::delHeader(const std::string& key) {
    __headers.erase(key);
}

void HttpRequest::delParam(const std::string& key) {
    __params.erase(key);
}

void HttpRequest::delCookie(const std::string& key) {
    __cookies.erase(key);
}

bool HttpRequest::hasHeader(const std::string& key, std::string* val) {
    auto it = __headers.find(key);
    if (it == __headers.end()) {
        return false;
    }
    if (val) {
        *val = it->second;
    }
    return true;
}

bool HttpRequest::hasParam(const std::string& key, std::string* val) {
    initQueryParam();
    initBodyParam();
    auto it = __params.find(key);
    if (it == __params.end()) {
        return false;
    }
    if (val) {
        *val = it->second;
    }
    return true;
}

bool HttpRequest::hasCookie(const std::string& key, std::string* val) {
    initCookies();
    auto it = __cookies.find(key);
    if (it == __cookies.end()) {
        return false;
    }
    if (val) {
        *val = it->second;
    }
    return true;
}

std::ostream& HttpRequest::dump(std::ostream& os) const {
    os << HttpMethodToString(__method) << " "
        << __path
        << (__query.empty() ? "" : "?")
        << __query
        << (__fragment.empty() ? "" : "#")
        << __fragment
        << " HTTP/"
        << ((uint32_t)(__version >> 4))
        << "."
        << ((uint32_t)(__version & 0x0F))
        << "\r\n";
    if (!__websocket) {
        os << "connection: " << (__close ? "close" : "keep-alive") << "\r\n";
    }
    for (auto& i : __headers) {
        if (!__websocket && strcasecmp(i.first.c_str(), "connection") == 0) {
            continue;
        }
        os << i.first << ": " << i.second << "\r\n";
    }

    if (!__body.empty()) {
        os << "content-length: " << __body.size() << "\r\n\r\n"
            << __body;
    }
    else {
        os << "\r\n";
    }
    return os;
}

std::string HttpRequest::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

void HttpRequest::init() {
    std::string conn = getHeader("connection");
    if (!conn.empty()) {
        if (strcasecmp(conn.c_str(), "keep-alive") == 0) {
            __close = false;
        }
        else {
            __close = true;
        }
    }
}

void HttpRequest::initParam() {
    initQueryParam();
    initBodyParam();
    initCookies();
}

void HttpRequest::initQueryParam() {
    if (__parserParamFlag & 0x1) {
        return;
    }

#define PARSE_PARAM(str, m, flag, trim) \
    size_t pos = 0; \
    do { \
        size_t last = pos; \
        pos = str.find('=', pos); \
        if(pos == std::string::npos) { \
            break; \
        } \
        size_t key = pos; \
        pos = str.find(flag, pos); \
        m.insert(std::make_pair(trim(str.substr(last, key - last)), \
                    sylar::StringUtil::UrlDecode(str.substr(key + 1, pos - key - 1)))); \
        if(pos == std::string::npos) { \
            break; \
        } \
        ++pos; \
    } while(true);

    PARSE_PARAM(__query, __params, '&', );
    __parserParamFlag |= 0x1;
}

void HttpRequest::initBodyParam() {
    if (__parserParamFlag & 0x2) {
        return;
    }
    std::string content_type = getHeader("content-type");
    if (strcasestr(content_type.c_str(), "application/x-www-form-urlencoded") == nullptr) {
        __parserParamFlag |= 0x2;
        return;
    }
    PARSE_PARAM(__body, __params, '&', );
    __parserParamFlag |= 0x2;
}

void HttpRequest::initCookies() {
    if (__parserParamFlag & 0x4) {
        return;
    }
    std::string cookie = getHeader("cookie");
    if (cookie.empty()) {
        __parserParamFlag |= 0x4;
        return;
    }
    PARSE_PARAM(cookie, __cookies, ';', sylar::StringUtil::Trim);
    __parserParamFlag |= 0x4;
}

//****************************************************************************
// HttpResponse
//****************************************************************************

HttpResponse::HttpResponse(uint8_t version, bool close) 
    :__status(HttpStatus::OK)
    , __version(version)
    , __close(close)
    , __websocket(false) {}

HttpStatus HttpResponse::getStatus() const {
	return __status;
}

uint8_t HttpResponse::getVersion() const {
	return __version;
}

const std::string& HttpResponse::getBody() const {
	return __body;
}

const std::string& HttpResponse::getReason() const {
	return __reason;
}

const HttpResponse::MapType& HttpResponse::getHeaders() const {
	return __headers;
}

void HttpResponse::setStatus(HttpStatus v) {
	__status = v;
}

void HttpResponse::setVersion(uint8_t v) {
	__version = v;
}

void HttpResponse::setBody(const std::string& v) {
	__body = v;
}

void HttpResponse::setReason(const std::string& v) {
	__reason = v;
}

void HttpResponse::setHeaders(const MapType& v) {
	__headers = v;
}

bool HttpResponse::isClose() const {
	return __close;
}

void HttpResponse::setClose(bool v) {
	__close = v;
}

bool HttpResponse::isWebsocket() const {
	return __websocket;
}

void HttpResponse::setWebsocket(bool v) {
	__websocket = v;
}

std::string HttpResponse::getHeader(const std::string& key, const std::string& def) const {
    auto it = __headers.find(key);
    return it == __headers.end() ? def : it->second;
}

void HttpResponse::setHeader(const std::string& key, const std::string& val) {
    __headers[key] = val;
}

void HttpResponse::delHeader(const std::string& key) {
    __headers.erase(key);
}

std::ostream& HttpResponse::dump(std::ostream& os) const {
    os << "HTTP/"
        << ((uint32_t)(__version >> 4))
        << "."
        << ((uint32_t)(__version & 0x0F))
        << " "
        << (uint32_t)__status
        << " "
        << (__reason.empty() ? HttpStatusToString(__status) : __reason)
        << "\r\n";

    for (auto& i : __headers) {
        if (!__websocket && strcasecmp(i.first.c_str(), "connection") == 0) {
            continue;
        }
        os << i.first << ": " << i.second << "\r\n";
    }
    for (auto& i : __cookies) {
        os << "Set-Cookie: " << i << "\r\n";
    }
    if (!__websocket) {
        os << "connection: " << (__close ? "close" : "keep-alive") << "\r\n";
    }
    if (!__body.empty()) {
        os << "content-length: " << __body.size() << "\r\n\r\n"
            << __body;
    }
    else {
        os << "\r\n";
    }
    return os;
}

std::string HttpResponse::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

void HttpResponse::setRedirect(const std::string& uri) {
    __status = HttpStatus::FOUND;
    setHeader("Location", uri);
}

void HttpResponse::setCookie(const std::string& key, const std::string& val,
							 time_t expired, const std::string& path,
							 const std::string& domain, bool secure) {
    std::stringstream ss;
    ss << key << "=" << val;
    if (expired > 0) {
        ss << ";expires=" << sylar::Time2Str(expired, "%a, %d %b %Y %H:%M:%S") << " GMT";
    }
    if (!domain.empty()) {
        ss << ";domain=" << domain;
    }
    if (!path.empty()) {
        ss << ";path=" << path;
    }
    if (secure) {
        ss << ";secure";
    }
    __cookies.push_back(ss.str());
}

//****************************************************************************
// 流式输出
//****************************************************************************

std::ostream& operator<<(std::ostream& os, const HttpRequest& req) {
    return req.dump(os);
}

std::ostream& operator<<(std::ostream& os, const HttpResponse& rsp) {
    return rsp.dump(os);
}

}; /* sylar::http */