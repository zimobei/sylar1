//*****************************************************************************
//
//
//   HTTP协议解析封装
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
// 前置声明
//****************************************************************************

class HttpRequestParser;
using HttpRequestParser_ptr = std::shared_ptr<HttpRequestParser>;

class HttpResponseParser;
using HttpResponseParser_ptr = std::shared_ptr<HttpResponseParser>;

//****************************************************************************
// HTTP请求解析类
//****************************************************************************

class HttpRequestParser {
private:
    http_parser __parser;       // http_parser
    HttpRequest_ptr __data;     // HttpRequest结构
    /// 错误码
    /// 1000: invalid method
    /// 1001: invalid version
    /// 1002: invalid field
    int m_error;
public:
    /*!
     * @brief 返回HttpRequest协议解析的缓存大小
     */
    static uint64_t GetHttpRequestBufferSize();

    /*!
     * @brief 返回HttpRequest协议的最大消息体大小
     */
    static uint64_t GetHttpRequestMaxBodySize();

    /*!
     * @brief 构造函数
     */
    HttpRequestParser();

    /*!
     * @brief 解析协议
     * @param data 协议文本内存
     * @param len 协议文本内存长度
     * @return 返回实际解析的长度,并且将已解析的数据移除
     */
    size_t execute(char* data, size_t len);

    /*!
     * @brief 是否解析完成
     */
    int isFinished();

    /*!
     * @brief 是否有错误
     */
    int hasError();

    /*!
     * @brief 返回HttpRequest结构体
     */
    HttpRequest_ptr getData() const;

    /*!
     * @brief 设置错误
     * @param v = 1000: invalid method
     * @param v = 1001: invalid version
     * @param v = 1002: invalid field
     */
    void setError(int v);

    /*!
     * @brief 获取消息体长度
     */
    uint64_t getContentLength();

    /*!
     * @brief 获取http_parser结构体
     */
    const http_parser& getParser() const;
};

//****************************************************************************
// Http响应解析结构体
//****************************************************************************

class HttpResponseParser {
private:
    httpclient_parser __parser;     // httpclient_parser
    HttpResponse_ptr __data;        // HttpResponse
    /// 错误码
    /// 1001: invalid version
    /// 1002: invalid field
    int m_error;
public:
    /*!
     * @brief 返回HTTP响应解析缓存大小
     */
    static uint64_t GetHttpResponseBufferSize();

    /*!
     * @brief 返回HTTP响应最大消息体大小
     */
    static uint64_t GetHttpResponseMaxBodySize();

    /*!
     * @brief 构造函数
     */
    HttpResponseParser();

    /*!
     * @brief 解析HTTP响应协议
     * @param data 协议数据内存
     * @param len 协议数据内存大小
     * @param chunck 是否在解析chunck
     * @return 返回实际解析的长度,并且移除已解析的数据
     */
    size_t execute(char* data, size_t len, bool chunck);

    /*!
     * @brief 是否解析完成
     */
    int isFinished();

    /*!
     * @brief 是否有错误
     */
    int hasError();

    /*!
     * @brief 返回HttpResponse
     */
    HttpResponse_ptr getData() const;

    /*!
     * @brief 设置错误码
     * @param v = 1000: invalid method
     * @param v = 1001: invalid version
     * @param v = 1002: invalid field
     */
    void setError(int v);

    /*!
     * @brief 获取消息体长度
     */
    uint64_t getContentLength();

    /*!
     * @brief 返回httpclient_parser
     */
    const httpclient_parser& getParser() const;
};

}; /* sylar::http */

#endif /* SYLAR_HTTP_PARSER_H */
