//*****************************************************************************
//
//
//   此头文件实现 HTTP 客户端类
//  
//
//*****************************************************************************

#ifndef SYLAR_HTTP_CONNECTION_H
#define SYLAR_HTTP_CONNECTION_H

#include "Stream.h"
#include "Http.h"
#include "Thread.h"
#include "Mutex.h"
#include "Uri.h"
#include <list>
#include <memory>
#include <string>
#include <sstream>

namespace sylar::http
{

//****************************************************************************
// 前置声明
//****************************************************************************

struct HttpResult;
using HttpResult_ptr = std::shared_ptr<HttpResult>;

class HttpConnection;
using HttpConnection_ptr = std::shared_ptr<HttpConnection>;

class HttpConnectionPool;
using HttpConnectionPool_ptr = std::shared_ptr<HttpConnectionPool>;

//****************************************************************************
// HTTP 响应结果
//****************************************************************************

struct HttpResult {
	enum class Error {
		OK = 0,                         // 正常
		INVALID_URL = 1,                // 非法URL
		INVALID_HOST = 2,               // 无法解析HOST
		CONNECT_FAIL = 3,               // 连接失败
		SEND_CLOSE_BY_PEER = 4,         // 连接被对端关闭
		SEND_SOCKET_ERROR = 5,          // 发送请求产生Socket错误
		TIMEOUT = 6,                    // 超时
		CREATE_SOCKET_ERROR = 7,        // 创建Socket失败
		POOL_GET_CONNECTION = 8,        // 从连接池中取连接失败
		POOL_INVALID_CONNECTION = 9,    // 无效的连接
	};

	int result;                         // 错误码
	HttpResponse_ptr response;         // HTTP响应结构体
	std::string error;                  // 错误描述

	HttpResult(int _result, HttpResponse_ptr _response, const std::string& _error);

	std::string toString() const;
};

//****************************************************************************
// HTTP 客户端类
//****************************************************************************

class HttpConnection : public SocketStream {
	friend class HttpConnectionPool;
private:
	uint64_t m_createTime = 0;
	uint64_t m_request = 0;
public:
	static HttpResult_ptr DoGet(const std::string& url, uint64_t timeout_ms, const std::map<std::string, std::string>& headers = {}, const std::string& body = "");

    static HttpResult_ptr DoGet(Uri_ptr uri, uint64_t timeout_ms, const std::map<std::string, std::string>& headers = {}, const std::string& body = "");

    static HttpResult_ptr DoPost(const std::string& url, uint64_t timeout_ms, const std::map<std::string, std::string>& headers = {}, const std::string& body = "");

    static HttpResult_ptr DoPost(Uri_ptr uri, uint64_t timeout_ms, const std::map<std::string, std::string>& headers = {}, const std::string& body = "");

    static HttpResult_ptr DoRequest(HttpMethod method, const std::string& url, uint64_t timeout_ms, const std::map<std::string, std::string>& headers = {}, const std::string& body = "");
	
	static HttpResult_ptr DoRequest(HttpMethod method, Uri_ptr uri, uint64_t timeout_ms, const std::map<std::string, std::string>& headers = {}, const std::string& body = "");

	static HttpResult_ptr DoRequest(HttpRequest_ptr req, Uri_ptr uri, uint64_t timeout_ms);

	HttpConnection(Socket_ptr sock, bool owner = true);

	~HttpConnection();

	HttpResponse_ptr recvResponse();

	int sendRequest(HttpRequest_ptr req);
};

//****************************************************************************
// HTTP 客户端池
//****************************************************************************

class HttpConnectionPool {
public:
	using MutexType = Mutex;
private:
	std::string m_host; // 主机
	std::string m_vhost; 
	uint32_t m_port; // 端口号
	uint32_t m_maxSize; // 连接最大数
	uint32_t m_maxAliveTime; // 连接时长
	uint32_t m_maxRequest; // 请求时长
	bool m_isHttps;

	MutexType m_mutex; // 锁
	std::list<HttpConnection*> m_conns; // HttpConnection 指针链表
	std::atomic<int32_t> m_total = { 0 }; // 连接的数量
private:
	static void ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool);
public:
	static HttpConnectionPool_ptr Create(const std::string& uri, const std::string& vhost, uint32_t max_size, uint32_t max_alive_time, uint32_t max_request);

	HttpConnectionPool(const std::string& host, const std::string& vhost, uint32_t port, bool is_https, uint32_t max_size, uint32_t max_alive_time, uint32_t max_request);

	HttpConnection_ptr getConnection();

    HttpResult_ptr doGet(const std::string& url, uint64_t timeout_ms, const std::map<std::string, std::string>& headers = {}, const std::string& body = "");

    HttpResult_ptr doGet(Uri_ptr uri, uint64_t timeout_ms, const std::map<std::string, std::string>& headers = {}, const std::string& body = "");

    HttpResult_ptr doPost(const std::string& url, uint64_t timeout_ms, const std::map<std::string, std::string>& headers = {}, const std::string& body = "");

    HttpResult_ptr doPost(Uri_ptr uri, uint64_t timeout_ms, const std::map<std::string, std::string>& headers = {}, const std::string& body = "");

    HttpResult_ptr doRequest(HttpMethod method, const std::string& url, uint64_t timeout_ms, const std::map<std::string, std::string>& headers = {}, const std::string& body = "");

    HttpResult_ptr doRequest(HttpMethod method, Uri_ptr uri, uint64_t timeout_ms, const std::map<std::string, std::string>& headers = {}, const std::string& body = "");

    HttpResult_ptr doRequest(HttpRequest_ptr req, uint64_t timeout_ms);
};

}; /* sylar::http */



#endif /* SYLAR_HTTP_CONNECTION_H */
