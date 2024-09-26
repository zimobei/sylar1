//*****************************************************************************
//
//
//   ��ͷ�ļ�ʵ�� HTTP �ͻ�����
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
// ǰ������
//****************************************************************************

struct HttpResult;
using HttpResult_ptr = std::shared_ptr<HttpResult>;

class HttpConnection;
using HttpConnection_ptr = std::shared_ptr<HttpConnection>;

class HttpConnectionPool;
using HttpConnectionPool_ptr = std::shared_ptr<HttpConnectionPool>;

//****************************************************************************
// HTTP ��Ӧ���
//****************************************************************************

struct HttpResult {
	enum class Error {
		OK = 0,                         // ����
		INVALID_URL = 1,                // �Ƿ�URL
		INVALID_HOST = 2,               // �޷�����HOST
		CONNECT_FAIL = 3,               // ����ʧ��
		SEND_CLOSE_BY_PEER = 4,         // ���ӱ��Զ˹ر�
		SEND_SOCKET_ERROR = 5,          // �����������Socket����
		TIMEOUT = 6,                    // ��ʱ
		CREATE_SOCKET_ERROR = 7,        // ����Socketʧ��
		POOL_GET_CONNECTION = 8,        // �����ӳ���ȡ����ʧ��
		POOL_INVALID_CONNECTION = 9,    // ��Ч������
	};

	int result;                         // ������
	HttpResponse_ptr response;         // HTTP��Ӧ�ṹ��
	std::string error;                  // ��������

	HttpResult(int _result, HttpResponse_ptr _response, const std::string& _error);

	std::string toString() const;
};

//****************************************************************************
// HTTP �ͻ�����
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
// HTTP �ͻ��˳�
//****************************************************************************

class HttpConnectionPool {
public:
	using MutexType = Mutex;
private:
	std::string m_host; // ����
	std::string m_vhost; 
	uint32_t m_port; // �˿ں�
	uint32_t m_maxSize; // ���������
	uint32_t m_maxAliveTime; // ����ʱ��
	uint32_t m_maxRequest; // ����ʱ��
	bool m_isHttps;

	MutexType m_mutex; // ��
	std::list<HttpConnection*> m_conns; // HttpConnection ָ������
	std::atomic<int32_t> m_total = { 0 }; // ���ӵ�����
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
