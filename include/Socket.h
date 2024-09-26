//*****************************************************************************
//
//
//   此头文件实现 socket 封装
//  
//
//*****************************************************************************

#ifndef SYLAR_SOCKET_H
#define SYLAR_SOCKET_H

#include <memory>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <boost/noncopyable.hpp>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "Address.h"

namespace sylar
{

//****************************************************************************
// 前置声明
//****************************************************************************

class Socket;
using Socket_ptr = std::shared_ptr<Socket>;
using Socket_weak = std::weak_ptr<Socket>;

class SSLSocket;
using SSLSocket_ptr = std::shared_ptr<SSLSocket>;

//****************************************************************************
// Socket封装类
//****************************************************************************

class Socket : public std::enable_shared_from_this<Socket>, boost::noncopyable {
public:
	enum Type {
		TCP = SOCK_STREAM,	// TCP 类型
		UDP = SOCK_DGRAM	// UDP 类型
	};
	enum Family {	
		IPv4 = AF_INET,		// IPv4 socket
		IPv6 = AF_INET6,	// IPv6 socket
		UNIX = AF_UNIX,		// Unix socket
	};
protected:  
    int __sock;						// socket句柄 
    int __family;					// 协议簇
    int __type;						// 类型
    int __protocol;					// 协议
    bool __isConnected;				// 是否连接
    Address_ptr __localAddress;		// 本地地址
    Address_ptr __remoteAddress;	// 远端地址
protected:
    /*!
     * @brief 初始化 socket
     */
    void initSock();

    /*!
     * @brief 创建 socket
     */
    void newSock();

    /*!
     * @brief 初始化 sock
     */
    virtual bool init(int sock);
public:
    /*!
     * @brief 创建TCP Socket(满足地址类型) 
     * @param address 地址
     */
    static Socket_ptr CreateTCP(Address_ptr address);

    /*!
     * @brief 创建UDP Socket(满足地址类型)
     * @param address 地址
     */
    static Socket_ptr CreateUDP(Address_ptr address);

    /*!
     * @brief 创建IPv4的TCP Socket
     */
    static Socket_ptr CreateTCPSocket();

    /*!
     * @brief 创建IPv4的UDP Socket
     */
    static Socket_ptr CreateUDPSocket();

    /*!
     * @brief 创建IPv6的TCP Socket
     */
    static Socket_ptr CreateTCPSocket6();

    /*!
     * @brief 创建IPv6的UDP Socket
     */
    static Socket_ptr CreateUDPSocket6();

    /*!
     * @brief 创建Unix的TCP Socket
     */
    static Socket_ptr CreateUnixTCPSocket();

    /*!
     * @brief 创建Unix的UDP Socket
     */
    static Socket_ptr CreateUnixUDPSocket();

    /*!
     * @brief Socket构造函数
     * @param family 协议簇
     * @param type 类型
     * @param protocol 协议
     */
    Socket(int family, int type, int protocol = 0);

    /*!
     * @brief 析构函数
     */
    virtual ~Socket();

    /*!
     * @brief 获取发送超时时间(毫秒)
     */
    int64_t getSendTimeout();

    /*!
     * @brief 设置发送超时时间(毫秒)
     */
    void setSendTimeout(int64_t v);

    /*!
     * @brief 获取接受超时时间(毫秒)
     */
    int64_t getRecvTimeout();

    /*!
     * @brief 设置接受超时时间(毫秒)
     */
    void setRecvTimeout(int64_t v);

    /*!
     * @brief 获取sockopt
     */
    bool getOption(int level, int option, void* result, socklen_t* len);

    /*!
     * @brief 获取sockopt模板
     */
    template<class T>
    bool getOption(int level, int option, T& result);

    /*!
     * @brief 设置sockopt
     */
    bool setOption(int level, int option, const void* result, socklen_t len);

    /*!
     * @brief 设置sockopt模板
     */
    template<class T>
    bool setOption(int level, int option, const T& value);

    /*!
     * @brief 接收connect链接
     * @return 成功返回新连接的socket,失败返回nullptr
     */
    virtual Socket_ptr accept();

    /*!
     * @brief 绑定地址
     * @param addr 地址
     * @return 是否绑定成功
     */
    virtual bool bind(const Address_ptr addr);

    /*!
     * @brief 连接地址
     * @param addr 目标地址
     * @param timeout_ms 超时时间(毫秒)
     */
    virtual bool connect(const Address_ptr addr, uint64_t timeout_ms = -1);

    /*!
     * @brief 重新连接地址
     * @param timeout_ms 超时时间(毫秒)
     * @return 
     */
    virtual bool reconnect(uint64_t timeout_ms = -1);

    /*!
     * @brief 监听socket
     * @param backlog 未完成连接队列的最大长度
     * @return 返回监听是否成功
     */
    virtual bool listen(int backlog = SOMAXCONN);

    /*!
     * @brief 关闭socket
     */
    virtual bool close();

    /*!
     * @brief 发送数据
     * @param buffer 待发送数据的内存
     * @param length 待发送数据的长度
     * @param flags 标志字
     * @return 
     *      @retval > 0 发送成功对应大小的数据
     *      @retval = 0 socket被关闭
     *      @retval < 0 socket出错
     */
    virtual int send(const void* buffer, size_t length, int flags = 0);

    /*!
     * @brief 发送数据
     * @param buffers 待发送数据的内存(iovec数组)
     * @param length 待发送数据的长度(iovec长度)
     * @param flags 标志字
     * @return 
     *      @retval > 0 发送成功对应大小的数据
     *      @retval = 0 socket被关闭
     *      @retval < 0 socket出错
     */
    virtual int send(const iovec* buffers, size_t length, int flags = 0);

    /*!
     * @brief 发送数据
     * @param buffer 待发送数据的内存
     * @param length 待发送数据的长度
     * @param to 发送的目标地址
     * @param flags 标志字
     * @return 
     *      @retval > 0 发送成功对应大小的数据
     *      @retval = 0 socket被关闭
     *      @retval < 0 socket出错
     */
    virtual int sendTo(const void* buffer, size_t length, const Address_ptr to, int flags = 0);

    /*!
     * @brief 发送数据
     * @param buffers 待发送数据的内存(iovec数组)
     * @param length 待发送数据的长度(iovec长度)
     * @param to 发送的目标地址
     * @param flags 标志字
     * @return 
     *      @retval > 0 发送成功对应大小的数据
     *      @retval = 0 socket被关闭
     *      @retval < 0 socket出错
     */
    virtual int sendTo(const iovec* buffers, size_t length, const Address_ptr to, int flags = 0);
    
    /*!
     * @brief 接受数据
     * @param buffer 接收数据的内存
     * @param length 接收数据的内存大小
     * @param flags 标志字
     * @return 
     *      @retval > 0 发送成功对应大小的数据
     *      @retval = 0 socket被关闭
     *      @retval < 0 socket出错
     */
    virtual int recv(void* buffer, size_t length, int flags = 0);

    /*!
     * @brief 接受数据
     * @param buffers 接收数据的内存(iovec数组)
     * @param length 接收数据的内存大小(iovec数组长度)
     * @param flags 标志字
     * @return 
     *      @retval > 0 发送成功对应大小的数据
     *      @retval = 0 socket被关闭
     *      @retval < 0 socket出错
     */
    virtual int recv(iovec* buffers, size_t length, int flags = 0);

    /*!
     * @brief 接受数据
     * @param buffer 接收数据的内存
     * @param length 接收数据的内存大小
     * @param from 发送端地址
     * @param flags 标志字
     * @return 
     *      @retval > 0 发送成功对应大小的数据
     *      @retval = 0 socket被关闭
     *      @retval < 0 socket出错
     */
    virtual int recvFrom(void* buffer, size_t length, Address_ptr from, int flags = 0);

    /*!
     * @brief 接受数据
     * @param buffers 接收数据的内存(iovec数组)
     * @param length 接收数据的内存大小(iovec数组长度)
     * @param from 发送端地址
     * @param flags 标志字
     * @return 
     *      @retval > 0 发送成功对应大小的数据
     *      @retval = 0 socket被关闭
     *      @retval < 0 socket出错
     */
    virtual int recvFrom(iovec* buffers, size_t length, Address_ptr from, int flags = 0);

    Address_ptr getRemoteAddress();

    Address_ptr getLocalAddress();

    int getFamily() const;

    int getType() const;

    int getProtocol() const;

    bool isConnected() const;

    bool isValid() const;

    int getError();

    virtual std::ostream& dump(std::ostream& os) const;

    virtual std::string toString() const;

    int getSocket() const;

    bool cancelRead();

    bool cancelWrite();

    bool cancelAccept();

    bool cancelAll();
};

//****************************************************************************
// 流式处理 socket
//****************************************************************************

std::ostream& operator<<(std::ostream& os, const Socket& sock);

//****************************************************************************
// Socket派生类
//****************************************************************************

class SSLSocket : public Socket {
private:
    std::shared_ptr<SSL_CTX> __ctx;
    std::shared_ptr<SSL> __ssl;
protected:
    virtual bool init(int sock) override;
public:
    static SSLSocket_ptr CreateTCP(Address_ptr address);
    static SSLSocket_ptr CreateTCPSocket();
    static SSLSocket_ptr CreateTCPSocket6();

    SSLSocket(int family, int type, int protocol = 0);
    virtual Socket_ptr accept() override;
    virtual bool bind(const Address_ptr addr) override;
    virtual bool connect(const Address_ptr addr, uint64_t timeout_ms = -1) override;
    virtual bool listen(int backlog = SOMAXCONN) override;
    virtual bool close() override;
    virtual int send(const void* buffer, size_t length, int flags = 0) override;
    virtual int send(const iovec* buffers, size_t length, int flags = 0) override;
    virtual int sendTo(const void* buffer, size_t length, const Address_ptr to, int flags = 0) override;
    virtual int sendTo(const iovec* buffers, size_t length, const Address_ptr to, int flags = 0) override;
    virtual int recv(void* buffer, size_t length, int flags = 0) override;
    virtual int recv(iovec* buffers, size_t length, int flags = 0) override;
    virtual int recvFrom(void* buffer, size_t length, Address_ptr from, int flags = 0) override;
    virtual int recvFrom(iovec* buffers, size_t length, Address_ptr from, int flags = 0) override;

    bool loadCertificates(const std::string& cert_file, const std::string& key_file);
    virtual std::ostream& dump(std::ostream& os) const override;
};

//****************************************************************************
// 模板类或函数的实现
//****************************************************************************

template<class T>
bool Socket::getOption(int level, int option, T& result) {
    socklen_t length = sizeof(T);
    return getOption(level, option, &result, &length);
}

template<class T>
bool Socket::setOption(int level, int option, const T& value) {
    return setOption(level, option, &value, sizeof(T));
}

}; /* sylar */

#endif /* SYLAR_SOCKET_H */
