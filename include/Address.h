//*****************************************************************************
//
//
//   ��ͷ�ļ�ʵ�ֶ������ַ�ķ�װ����
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
// ǰ������
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
// �����ַ�Ļ���,������
//****************************************************************************

class Address {
public:
    /*!
     * @brief ͨ��sockaddrָ�봴��Address
     * @param addr sockaddrָ��
     * @param addrlen sockaddr�ĳ���
     * @return ���غ�sockaddr��ƥ���Address,ʧ�ܷ���nullptr
     */
    static Address_ptr Create(const sockaddr* addr, socklen_t addrlen);

    /*!
     * @brief ͨ��host��ַ���ض�Ӧ����������Address
     * @param result ��������������Address
     * @param host ����,����������.����: www.sylar.top[:80] (������Ϊ��ѡ����)
     * @param family Э����(AF_INT, AF_INT6, AF_UNIX)
     * @param type socketl����SOCK_STREAM��SOCK_DGRAM ��
     * @param protocol Э��,IPPROTO_TCP��IPPROTO_UDP ��
     * @return �����Ƿ�ת���ɹ�
     */
    static bool Lookup(std::vector<Address_ptr>& result, const std::string& host,
                       int family = AF_INET, int type = 0, int protocol = 0);

    /*!
     * @brief ͨ��host��ַ���ض�Ӧ����������Address
     * @param host ����,����������.����: www.sylar.top[:80] (������Ϊ��ѡ����)
     * @param family Э����(AF_INT, AF_INT6, AF_UNIX)
     * @param type socketl����SOCK_STREAM��SOCK_DGRAM ��
     * @param protocol Э��,IPPROTO_TCP��IPPROTO_UDP ��
     * @return ������������������Address,ʧ�ܷ���nullptr
     */
    static Address_ptr LookupAny(const std::string& host,
                                  int family = AF_INET, int type = 0, int protocol = 0);

    /*!
     * @brief  ͨ��host��ַ���ض�Ӧ����������IPAddress
     * @param host ����,����������.����: www.sylar.top[:80] (������Ϊ��ѡ����)
     * @param family Э����(AF_INT, AF_INT6, AF_UNIX)
     * @param type socketl����SOCK_STREAM��SOCK_DGRAM ��
     * @param protocol Э��,IPPROTO_TCP��IPPROTO_UDP ��
     * @return ������������������IPAddress,ʧ�ܷ���nullptr
     */
    static IPAddress_ptr LookupAnyIPAddress(const std::string& host,
                                                         int family = AF_INET, int type = 0, int protocol = 0);

    /*!
     * @brief ���ر�������������<������, ��ַ, ��������λ��>
     * @param result ���汾�����е�ַ
     * @param family Э����(AF_INT, AF_INT6, AF_UNIX)
     * @return �Ƿ��ȡ�ɹ�
     */
    static bool GetInterfaceAddresses(std::multimap<std::string, 
                                      std::pair<Address_ptr, uint32_t>>& result,
                                      int family = AF_INET);

    /*!
     * @brief ��ȡָ�������ĵ�ַ����������λ��
     * @param result ����ָ���������е�ַ
     * @param iface ��������
     * @param family Э����(AF_INT, AF_INT6, AF_UNIX)
     * @return �Ƿ��ȡ�ɹ�
     */
    static bool GetInterfaceAddresses(std::vector<std::pair<Address_ptr, uint32_t> >& result
                                      , const std::string& iface, int family = AF_INET);
    /*!
     * @brief ����������
     */
    virtual ~Address();

    /*!
     * @brief ����Э���
     */
    int getFamily() const;

    /*!
     * @brief ����sockaddrָ��,ֻ��
     */
    virtual const sockaddr* getAddr() const = 0;

    /*!
     * @brief ����sockaddrָ��,��д
     */
    virtual sockaddr* getAddr() = 0;

    /*!
     * @brief ����sockaddr�ĳ���
     */
    virtual socklen_t getAddrLen() const = 0;

    /*!
     * @brief �ɶ��������ַ
     */
    virtual std::ostream& insert(std::ostream& os) const = 0;

    /*!
     * @brief ���ؿɶ����ַ���
     */
    std::string toString() const;

    /*!
     * @brief С�ںűȽϺ���
     */
    bool operator<(const Address& rhs) const;

    /*!
     * @brief ���ں���
     */
    bool operator==(const Address& rhs) const;
};

//****************************************************************************
// ��ʽ���Address
//****************************************************************************

std::ostream& operator<<(std::ostream& os, const Address& addr);

//****************************************************************************
// IP��ַ�Ļ���
//****************************************************************************

class IPAddress : public Address {
public:
    /*!
     * @brief ͨ������,IP,������������IPAddress
     * @param address ����,IP,����������.����: www.sylar.top
     * @param port �˿ں�
     * @return ���óɹ�����IPAddress,ʧ�ܷ���nullptr
     */
    static IPAddress_ptr Create(const char* address, uint16_t port = 0);

    /*!
     * @brief ��ȡ�õ�ַ�Ĺ㲥��ַ
     * @param prefix_len ��������λ��
     * @return ���óɹ�����IPAddress,ʧ�ܷ���nullptr
     */
    virtual IPAddress_ptr broadcastAddress(uint32_t prefix_len) = 0;

    /*!
     * @brief ��ȡ�õ�ַ������
     * @param prefix_len ��������λ��
     * @return ���óɹ�����IPAddress,ʧ�ܷ���nullptr
     */
    virtual IPAddress_ptr networkAddress(uint32_t prefix_len) = 0;

    /*!
     * @brief ��ȡ���������ַ
     * @param prefix_len ��������λ��
     * @return ���óɹ�����IPAddress,ʧ�ܷ���nullptr
     */
    virtual IPAddress_ptr subnetMask(uint32_t prefix_len) = 0;

    /*!
     * @brief ���ض˿ں�
     */
    virtual uint32_t getPort() const = 0;

    /*!
     * @brief ���ö˿ں�
     */
    virtual void setPort(uint16_t v) = 0;
};

//****************************************************************************
// IPv4��ַ
//****************************************************************************

class IPv4Address : public IPAddress {
private:
    sockaddr_in __address;
public:
    /*!
     * @brief ʹ�õ��ʮ���Ƶ�ַ����IPv4Address
     * @param address ���ʮ���Ƶ�ַ,��:192.168.1.1
     * @param port �˿ں�
     * @return ����IPv4Address,ʧ�ܷ���nullptr
     */
    static IPv4Address_ptr Create(const char* address, uint16_t port = 0);

    /*!
     * @brief ͨ��sockaddr_in����IPv4Address
     * @param address sockaddr_in�ṹ��
     */
    IPv4Address(const sockaddr_in& address);

    /*!
     * @brief ͨ�������Ƶ�ַ����IPv4Address
     * @param address �����Ƶ�ַaddress
     * @param port �˿ں�
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
// IPv6��ַ
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
// UnixSocket��ַ
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
// δ֪��ַ
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
