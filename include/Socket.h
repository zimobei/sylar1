//*****************************************************************************
//
//
//   ��ͷ�ļ�ʵ�� socket ��װ
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
// ǰ������
//****************************************************************************

class Socket;
using Socket_ptr = std::shared_ptr<Socket>;
using Socket_weak = std::weak_ptr<Socket>;

class SSLSocket;
using SSLSocket_ptr = std::shared_ptr<SSLSocket>;

//****************************************************************************
// Socket��װ��
//****************************************************************************

class Socket : public std::enable_shared_from_this<Socket>, boost::noncopyable {
public:
	enum Type {
		TCP = SOCK_STREAM,	// TCP ����
		UDP = SOCK_DGRAM	// UDP ����
	};
	enum Family {	
		IPv4 = AF_INET,		// IPv4 socket
		IPv6 = AF_INET6,	// IPv6 socket
		UNIX = AF_UNIX,		// Unix socket
	};
protected:  
    int __sock;						// socket��� 
    int __family;					// Э���
    int __type;						// ����
    int __protocol;					// Э��
    bool __isConnected;				// �Ƿ�����
    Address_ptr __localAddress;		// ���ص�ַ
    Address_ptr __remoteAddress;	// Զ�˵�ַ
protected:
    /*!
     * @brief ��ʼ�� socket
     */
    void initSock();

    /*!
     * @brief ���� socket
     */
    void newSock();

    /*!
     * @brief ��ʼ�� sock
     */
    virtual bool init(int sock);
public:
    /*!
     * @brief ����TCP Socket(�����ַ����) 
     * @param address ��ַ
     */
    static Socket_ptr CreateTCP(Address_ptr address);

    /*!
     * @brief ����UDP Socket(�����ַ����)
     * @param address ��ַ
     */
    static Socket_ptr CreateUDP(Address_ptr address);

    /*!
     * @brief ����IPv4��TCP Socket
     */
    static Socket_ptr CreateTCPSocket();

    /*!
     * @brief ����IPv4��UDP Socket
     */
    static Socket_ptr CreateUDPSocket();

    /*!
     * @brief ����IPv6��TCP Socket
     */
    static Socket_ptr CreateTCPSocket6();

    /*!
     * @brief ����IPv6��UDP Socket
     */
    static Socket_ptr CreateUDPSocket6();

    /*!
     * @brief ����Unix��TCP Socket
     */
    static Socket_ptr CreateUnixTCPSocket();

    /*!
     * @brief ����Unix��UDP Socket
     */
    static Socket_ptr CreateUnixUDPSocket();

    /*!
     * @brief Socket���캯��
     * @param family Э���
     * @param type ����
     * @param protocol Э��
     */
    Socket(int family, int type, int protocol = 0);

    /*!
     * @brief ��������
     */
    virtual ~Socket();

    /*!
     * @brief ��ȡ���ͳ�ʱʱ��(����)
     */
    int64_t getSendTimeout();

    /*!
     * @brief ���÷��ͳ�ʱʱ��(����)
     */
    void setSendTimeout(int64_t v);

    /*!
     * @brief ��ȡ���ܳ�ʱʱ��(����)
     */
    int64_t getRecvTimeout();

    /*!
     * @brief ���ý��ܳ�ʱʱ��(����)
     */
    void setRecvTimeout(int64_t v);

    /*!
     * @brief ��ȡsockopt
     */
    bool getOption(int level, int option, void* result, socklen_t* len);

    /*!
     * @brief ��ȡsockoptģ��
     */
    template<class T>
    bool getOption(int level, int option, T& result);

    /*!
     * @brief ����sockopt
     */
    bool setOption(int level, int option, const void* result, socklen_t len);

    /*!
     * @brief ����sockoptģ��
     */
    template<class T>
    bool setOption(int level, int option, const T& value);

    /*!
     * @brief ����connect����
     * @return �ɹ����������ӵ�socket,ʧ�ܷ���nullptr
     */
    virtual Socket_ptr accept();

    /*!
     * @brief �󶨵�ַ
     * @param addr ��ַ
     * @return �Ƿ�󶨳ɹ�
     */
    virtual bool bind(const Address_ptr addr);

    /*!
     * @brief ���ӵ�ַ
     * @param addr Ŀ���ַ
     * @param timeout_ms ��ʱʱ��(����)
     */
    virtual bool connect(const Address_ptr addr, uint64_t timeout_ms = -1);

    /*!
     * @brief �������ӵ�ַ
     * @param timeout_ms ��ʱʱ��(����)
     * @return 
     */
    virtual bool reconnect(uint64_t timeout_ms = -1);

    /*!
     * @brief ����socket
     * @param backlog δ������Ӷ��е���󳤶�
     * @return ���ؼ����Ƿ�ɹ�
     */
    virtual bool listen(int backlog = SOMAXCONN);

    /*!
     * @brief �ر�socket
     */
    virtual bool close();

    /*!
     * @brief ��������
     * @param buffer ���������ݵ��ڴ�
     * @param length ���������ݵĳ���
     * @param flags ��־��
     * @return 
     *      @retval > 0 ���ͳɹ���Ӧ��С������
     *      @retval = 0 socket���ر�
     *      @retval < 0 socket����
     */
    virtual int send(const void* buffer, size_t length, int flags = 0);

    /*!
     * @brief ��������
     * @param buffers ���������ݵ��ڴ�(iovec����)
     * @param length ���������ݵĳ���(iovec����)
     * @param flags ��־��
     * @return 
     *      @retval > 0 ���ͳɹ���Ӧ��С������
     *      @retval = 0 socket���ر�
     *      @retval < 0 socket����
     */
    virtual int send(const iovec* buffers, size_t length, int flags = 0);

    /*!
     * @brief ��������
     * @param buffer ���������ݵ��ڴ�
     * @param length ���������ݵĳ���
     * @param to ���͵�Ŀ���ַ
     * @param flags ��־��
     * @return 
     *      @retval > 0 ���ͳɹ���Ӧ��С������
     *      @retval = 0 socket���ر�
     *      @retval < 0 socket����
     */
    virtual int sendTo(const void* buffer, size_t length, const Address_ptr to, int flags = 0);

    /*!
     * @brief ��������
     * @param buffers ���������ݵ��ڴ�(iovec����)
     * @param length ���������ݵĳ���(iovec����)
     * @param to ���͵�Ŀ���ַ
     * @param flags ��־��
     * @return 
     *      @retval > 0 ���ͳɹ���Ӧ��С������
     *      @retval = 0 socket���ر�
     *      @retval < 0 socket����
     */
    virtual int sendTo(const iovec* buffers, size_t length, const Address_ptr to, int flags = 0);
    
    /*!
     * @brief ��������
     * @param buffer �������ݵ��ڴ�
     * @param length �������ݵ��ڴ��С
     * @param flags ��־��
     * @return 
     *      @retval > 0 ���ͳɹ���Ӧ��С������
     *      @retval = 0 socket���ر�
     *      @retval < 0 socket����
     */
    virtual int recv(void* buffer, size_t length, int flags = 0);

    /*!
     * @brief ��������
     * @param buffers �������ݵ��ڴ�(iovec����)
     * @param length �������ݵ��ڴ��С(iovec���鳤��)
     * @param flags ��־��
     * @return 
     *      @retval > 0 ���ͳɹ���Ӧ��С������
     *      @retval = 0 socket���ر�
     *      @retval < 0 socket����
     */
    virtual int recv(iovec* buffers, size_t length, int flags = 0);

    /*!
     * @brief ��������
     * @param buffer �������ݵ��ڴ�
     * @param length �������ݵ��ڴ��С
     * @param from ���Ͷ˵�ַ
     * @param flags ��־��
     * @return 
     *      @retval > 0 ���ͳɹ���Ӧ��С������
     *      @retval = 0 socket���ر�
     *      @retval < 0 socket����
     */
    virtual int recvFrom(void* buffer, size_t length, Address_ptr from, int flags = 0);

    /*!
     * @brief ��������
     * @param buffers �������ݵ��ڴ�(iovec����)
     * @param length �������ݵ��ڴ��С(iovec���鳤��)
     * @param from ���Ͷ˵�ַ
     * @param flags ��־��
     * @return 
     *      @retval > 0 ���ͳɹ���Ӧ��С������
     *      @retval = 0 socket���ر�
     *      @retval < 0 socket����
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
// ��ʽ���� socket
//****************************************************************************

std::ostream& operator<<(std::ostream& os, const Socket& sock);

//****************************************************************************
// Socket������
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
// ģ���������ʵ��
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
