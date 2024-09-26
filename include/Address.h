//*****************************************************************************
//
//
//   此头文件实现对网络地址的封装功能
//  
//
//*****************************************************************************

#ifndef SYLAR_ADDRESS_H
#define SYLAR_ADDRESS_H

#include <memory>
#include <vector>
#include <map>
#include <ostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>

namespace sylar
{

//****************************************************************************
// 前置声明
//****************************************************************************

class Address;
using Address_ptr = std::shared_ptr<Address>;

class IPAddress;
using IPAddress_ptr = std::shared_ptr<IPAddress>;

class IPv4Address;
using IPv4Address_ptr = std::shared_ptr<IPv4Address>;

class IPv6Address;
using IPv6Address_ptr = std::shared_ptr<IPv6Address>;

class UnixAddress;
using UnixAddress_ptr = std::shared_ptr<UnixAddress>;

class UnknownAddress;
using UnknownAddress_ptr = std::shared_ptr<UnknownAddress>;

//****************************************************************************
// 网络地址的基类,抽象类
//****************************************************************************

class Address {
public:
    /*!
     * @brief 通过sockaddr指针创建Address
     * @param addr sockaddr指针
     * @param addrlen sockaddr的长度
     * @return 返回和sockaddr相匹配的Address,失败返回nullptr
     */
    static Address_ptr Create(const sockaddr* addr, socklen_t addrlen);

    /*!
     * @brief 通过host地址返回对应条件的所有Address
     * @param result 保存满足条件的Address
     * @param host 域名,服务器名等.举例: www.sylar.top[:80] (方括号为可选内容)
     * @param family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @param type socketl类型SOCK_STREAM、SOCK_DGRAM 等
     * @param protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
     * @return 返回是否转换成功
     */
    static bool Lookup(std::vector<Address_ptr>& result, const std::string& host,
                       int family = AF_INET, int type = 0, int protocol = 0);

    /*!
     * @brief 通过host地址返回对应条件的任意Address
     * @param host 域名,服务器名等.举例: www.sylar.top[:80] (方括号为可选内容)
     * @param family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @param type socketl类型SOCK_STREAM、SOCK_DGRAM 等
     * @param protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
     * @return 返回满足条件的任意Address,失败返回nullptr
     */
    static Address_ptr LookupAny(const std::string& host,
                                  int family = AF_INET, int type = 0, int protocol = 0);

    /*!
     * @brief  通过host地址返回对应条件的任意IPAddress
     * @param host 域名,服务器名等.举例: www.sylar.top[:80] (方括号为可选内容)
     * @param family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @param type socketl类型SOCK_STREAM、SOCK_DGRAM 等
     * @param protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
     * @return 返回满足条件的任意IPAddress,失败返回nullptr
     */
    static IPAddress_ptr LookupAnyIPAddress(const std::string& host,
                                                         int family = AF_INET, int type = 0, int protocol = 0);

    /*!
     * @brief 返回本机所有网卡的<网卡名, 地址, 子网掩码位数>
     * @param result 保存本机所有地址
     * @param family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @return 是否获取成功
     */
    static bool GetInterfaceAddresses(std::multimap<std::string, 
                                      std::pair<Address_ptr, uint32_t>>& result,
                                      int family = AF_INET);

    /*!
     * @brief 获取指定网卡的地址和子网掩码位数
     * @param result 保存指定网卡所有地址
     * @param iface 网卡名称
     * @param family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @return 是否获取成功
     */
    static bool GetInterfaceAddresses(std::vector<std::pair<Address_ptr, uint32_t> >& result
                                      , const std::string& iface, int family = AF_INET);
    /*!
     * @brief 虚析构函数
     */
    virtual ~Address();

    /*!
     * @brief 返回协议簇
     */
    int getFamily() const;

    /*!
     * @brief 返回sockaddr指针,只读
     */
    virtual const sockaddr* getAddr() const = 0;

    /*!
     * @brief 返回sockaddr指针,读写
     */
    virtual sockaddr* getAddr() = 0;

    /*!
     * @brief 返回sockaddr的长度
     */
    virtual socklen_t getAddrLen() const = 0;

    /*!
     * @brief 可读性输出地址
     */
    virtual std::ostream& insert(std::ostream& os) const = 0;

    /*!
     * @brief 返回可读性字符串
     */
    std::string toString() const;

    /*!
     * @brief 小于号比较函数
     */
    bool operator<(const Address& rhs) const;

    /*!
     * @brief 等于函数
     */
    bool operator==(const Address& rhs) const;
};

//****************************************************************************
// 流式输出Address
//****************************************************************************

std::ostream& operator<<(std::ostream& os, const Address& addr);

//****************************************************************************
// IP地址的基类
//****************************************************************************

class IPAddress : public Address {
public:
    /*!
     * @brief 通过域名,IP,服务器名创建IPAddress
     * @param address 域名,IP,服务器名等.举例: www.sylar.top
     * @param port 端口号
     * @return 调用成功返回IPAddress,失败返回nullptr
     */
    static IPAddress_ptr Create(const char* address, uint16_t port = 0);

    /*!
     * @brief 获取该地址的广播地址
     * @param prefix_len 子网掩码位数
     * @return 调用成功返回IPAddress,失败返回nullptr
     */
    virtual IPAddress_ptr broadcastAddress(uint32_t prefix_len) = 0;

    /*!
     * @brief 获取该地址的网段
     * @param prefix_len 子网掩码位数
     * @return 调用成功返回IPAddress,失败返回nullptr
     */
    virtual IPAddress_ptr networkAddress(uint32_t prefix_len) = 0;

    /*!
     * @brief 获取子网掩码地址
     * @param prefix_len 子网掩码位数
     * @return 调用成功返回IPAddress,失败返回nullptr
     */
    virtual IPAddress_ptr subnetMask(uint32_t prefix_len) = 0;

    /*!
     * @brief 返回端口号
     */
    virtual uint32_t getPort() const = 0;

    /*!
     * @brief 设置端口号
     */
    virtual void setPort(uint16_t v) = 0;
};

//****************************************************************************
// IPv4地址
//****************************************************************************

class IPv4Address : public IPAddress {
private:
    sockaddr_in __address;
public:
    /*!
     * @brief 使用点分十进制地址创建IPv4Address
     * @param address 点分十进制地址,如:192.168.1.1
     * @param port 端口号
     * @return 返回IPv4Address,失败返回nullptr
     */
    static IPv4Address_ptr Create(const char* address, uint16_t port = 0);

    /*!
     * @brief 通过sockaddr_in构造IPv4Address
     * @param address sockaddr_in结构体
     */
    IPv4Address(const sockaddr_in& address);

    /*!
     * @brief 通过二进制地址构造IPv4Address
     * @param address 二进制地址address
     * @param port 端口号
     */
    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress_ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress_ptr networkAddress(uint32_t prefix_len) override;
    IPAddress_ptr subnetMask(uint32_t prefix_len) override;
    uint32_t getPort() const override;
    void setPort(uint16_t v) override;
};

//****************************************************************************
// IPv6地址
//****************************************************************************

class IPv6Address : public IPAddress {
private:
    sockaddr_in6 __address;
public:
    static IPv6Address_ptr Create(const char* address, uint16_t port = 0);

    IPv6Address();

    IPv6Address(const sockaddr_in6& address);

    IPv6Address(const uint8_t address[16], uint16_t port = 0);

    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress_ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress_ptr networkAddress(uint32_t prefix_len) override;
    IPAddress_ptr subnetMask(uint32_t prefix_len) override;
    uint32_t getPort() const override;
    void setPort(uint16_t v) override;
};

//****************************************************************************
// UnixSocket地址
//****************************************************************************

class UnixAddress : public Address {
private:
    sockaddr_un __address;
    socklen_t __length;
public:
    UnixAddress();

    UnixAddress(const std::string& path);

    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    void setAddrLen(uint32_t v);
    std::string getPath() const;
    std::ostream& insert(std::ostream& os) const override;
};

//****************************************************************************
// 未知地址
//****************************************************************************

class UnknownAddress : public Address {
private:
    sockaddr __address;
public:
    UnknownAddress(int family);
    UnknownAddress(const sockaddr& addr);
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
};

}; /* sylar */

#endif /* SYLAR_ADDRESS_H */
