//*****************************************************************************
//
//
//   HTTPЭ�������װ
//  
//
//*****************************************************************************

#ifndef SYLAR_HTTP_PARSER_H
#define SYLAR_HTTP_PARSER_H

#include "Http.h"
#include "http11/http11_parser.h"
#include "http11/httpclient_parser.h"

namespace sylar::http
{

//****************************************************************************
// ǰ������
//****************************************************************************

class HttpRequestParser;
using HttpRequestParser_ptr = std::shared_ptr<HttpRequestParser>;

class HttpResponseParser;
using HttpResponseParser_ptr = std::shared_ptr<HttpResponseParser>;

//****************************************************************************
// HTTP���������
//****************************************************************************

class HttpRequestParser {
private:
    http_parser __parser;       // http_parser
    HttpRequest_ptr __data;     // HttpRequest�ṹ
    /// ������
    /// 1000: invalid method
    /// 1001: invalid version
    /// 1002: invalid field
    int m_error;
public:
    /*!
     * @brief ����HttpRequestЭ������Ļ����С
     */
    static uint64_t GetHttpRequestBufferSize();

    /*!
     * @brief ����HttpRequestЭ��������Ϣ���С
     */
    static uint64_t GetHttpRequestMaxBodySize();

    /*!
     * @brief ���캯��
     */
    HttpRequestParser();

    /*!
     * @brief ����Э��
     * @param data Э���ı��ڴ�
     * @param len Э���ı��ڴ泤��
     * @return ����ʵ�ʽ����ĳ���,���ҽ��ѽ����������Ƴ�
     */
    size_t execute(char* data, size_t len);

    /*!
     * @brief �Ƿ�������
     */
    int isFinished();

    /*!
     * @brief �Ƿ��д���
     */
    int hasError();

    /*!
     * @brief ����HttpRequest�ṹ��
     */
    HttpRequest_ptr getData() const;

    /*!
     * @brief ���ô���
     * @param v = 1000: invalid method
     * @param v = 1001: invalid version
     * @param v = 1002: invalid field
     */
    void setError(int v);

    /*!
     * @brief ��ȡ��Ϣ�峤��
     */
    uint64_t getContentLength();

    /*!
     * @brief ��ȡhttp_parser�ṹ��
     */
    const http_parser& getParser() const;
};

//****************************************************************************
// Http��Ӧ�����ṹ��
//****************************************************************************

class HttpResponseParser {
private:
    httpclient_parser __parser;     // httpclient_parser
    HttpResponse_ptr __data;        // HttpResponse
    /// ������
    /// 1001: invalid version
    /// 1002: invalid field
    int m_error;
public:
    /*!
     * @brief ����HTTP��Ӧ���������С
     */
    static uint64_t GetHttpResponseBufferSize();

    /*!
     * @brief ����HTTP��Ӧ�����Ϣ���С
     */
    static uint64_t GetHttpResponseMaxBodySize();

    /*!
     * @brief ���캯��
     */
    HttpResponseParser();

    /*!
     * @brief ����HTTP��ӦЭ��
     * @param data Э�������ڴ�
     * @param len Э�������ڴ��С
     * @param chunck �Ƿ��ڽ���chunck
     * @return ����ʵ�ʽ����ĳ���,�����Ƴ��ѽ���������
     */
    size_t execute(char* data, size_t len, bool chunck);

    /*!
     * @brief �Ƿ�������
     */
    int isFinished();

    /*!
     * @brief �Ƿ��д���
     */
    int hasError();

    /*!
     * @brief ����HttpResponse
     */
    HttpResponse_ptr getData() const;

    /*!
     * @brief ���ô�����
     * @param v = 1000: invalid method
     * @param v = 1001: invalid version
     * @param v = 1002: invalid field
     */
    void setError(int v);

    /*!
     * @brief ��ȡ��Ϣ�峤��
     */
    uint64_t getContentLength();

    /*!
     * @brief ����httpclient_parser
     */
    const httpclient_parser& getParser() const;
};

}; /* sylar::http */

#endif /* SYLAR_HTTP_PARSER_H */
