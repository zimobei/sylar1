#include "Address.h"
#include "Endian.h"
#include "Log.h"
#include <sstream>

namespace sylar
{

//****************************************************************************
// Other
//****************************************************************************

template<class T>
static T CreateMask(uint32_t bits) {
    return (1 << (sizeof(T) * 8 - bits)) - 1;
}

template<class T>
static uint32_t CountBytes(T value) {
    uint32_t result = 0;
    for (; value; ++result) {
        value &= value - 1;
    }
    return result;
}

//****************************************************************************
// 流式输出Address
//****************************************************************************

std::ostream& operator<<(std::ostream& os, const Address& addr) {
    return addr.insert(os);
}

//****************************************************************************
// Address
//****************************************************************************

Address_ptr Address::Create(const sockaddr* addr, socklen_t addrlen) {
    if (addr == nullptr) return nullptr;

    Address_ptr result;
    switch (addr->sa_family) {
        case AF_INET:
            result.reset(new IPv4Address(*(const sockaddr_in*)addr));
            break;
        case AF_INET6:
            result.reset(new IPv6Address(*(const sockaddr_in6*)addr));
            break;
        default:
            result.reset(new UnknownAddress(*addr));
            break;
    }
    return result;
}

bool Address::Lookup(std::vector<Address_ptr>& result, const std::string& host,
                     int family, int type, int protocol) {
    addrinfo hints, * results, * next;
    hints.ai_flags = 0;
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    hints.ai_addrlen = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    std::string node;
    const char* service = NULL;

    //检查 ipv6address serivce
    if (!host.empty() && host[0] == '[') {
        const char* endipv6 = (const char*)memchr(host.c_str() + 1, ']', host.size() - 1);
        if (endipv6) {
            //TODO check out of range
            if (*(endipv6 + 1) == ':') {
                service = endipv6 + 2;
            }
            node = host.substr(1, endipv6 - host.c_str() - 1);
        }
    }

    //检查 node serivce
    if (node.empty()) {
        service = (const char*)memchr(host.c_str(), ':', host.size());
        if (service) {
            if (!memchr(service + 1, ':', host.c_str() + host.size() - service - 1)) {
                node = host.substr(0, service - host.c_str());
                ++service;
            }
        }
    }

    if (node.empty()) {
        node = host;
    }
    int error = getaddrinfo(node.c_str(), service, &hints, &results);
    if (error) {
        SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) 
            << "Address::Lookup getaddress( " << host << ", "
            << family << ", " 
            << type << ") err = " 
            << error << " errstr = "
            << gai_strerror(error);
        return false;
    }

    next = results;
    while (next) {
        result.push_back(Create(next->ai_addr, (socklen_t)next->ai_addrlen));
        next = next->ai_next;
    }

    freeaddrinfo(results);
    return !result.empty();
}

Address_ptr Address::LookupAny(const std::string& host,
                               int family, int type, int protocol) {
    std::vector<Address_ptr> result;
    if (Lookup(result, host, family, type, protocol)) {
        return result[0];
    }
    return nullptr;
}

IPAddress_ptr Address::LookupAnyIPAddress(const std::string& host,
                                          int family, int type, int protocol) {
    std::vector<Address_ptr> result;
    if (Lookup(result, host, family, type, protocol)) {
        for (auto& it : result) {
            IPAddress_ptr var = std::dynamic_pointer_cast<IPAddress>(it);
            if (var) return var;
        }
    }
    return nullptr;
}

bool Address::GetInterfaceAddresses(std::multimap<std::string, std::pair<Address_ptr, uint32_t>>& result,
                                    int family) {
    struct ifaddrs* next, * results;
    if (getifaddrs(&results) != 0) {
        SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) 
            << "Address::GetInterfaceAddresses getifaddrs "
            << " err = " << errno 
            << " errstr = " << strerror(errno);
        return false;
    }

    try {
        for (next = results; next; next = next->ifa_next) {
            Address_ptr addr;
            uint32_t prefix_len = ~0u;
            if (family != AF_UNSPEC && family != next->ifa_addr->sa_family) {
                continue;
            }
            switch (next->ifa_addr->sa_family) {
                case AF_INET:
                {
                    addr = Create(next->ifa_addr, sizeof(sockaddr_in));
                    uint32_t netmask = ((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;
                    prefix_len = CountBytes(netmask);
                }
                break;
                case AF_INET6:
                {
                    addr = Create(next->ifa_addr, sizeof(sockaddr_in6));
                    in6_addr& netmask = ((sockaddr_in6*)next->ifa_netmask)->sin6_addr;
                    prefix_len = 0;
                    for (int i = 0; i < 16; ++i) {
                        prefix_len += CountBytes(netmask.s6_addr[i]);
                    }
                }
                break;
                default:
                    break;
            }

            if (addr) {
                result.insert(std::make_pair(next->ifa_name, std::make_pair(addr, prefix_len)));
            }
        }
    }
    catch (...) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Address::GetInterfaceAddresses exception";
        freeifaddrs(results);
        return false;
    }
    freeifaddrs(results);
    return true;
}

bool Address::GetInterfaceAddresses(std::vector<std::pair<Address_ptr, uint32_t> >& result, 
                                    const std::string& iface, int family) {
    if (iface.empty() || iface == "*") {
        if (family == AF_INET || family == AF_UNSPEC) {
            result.push_back(std::make_pair(Address_ptr(new IPv4Address()), 0u));
        }
        if (family == AF_INET6 || family == AF_UNSPEC) {
            result.push_back(std::make_pair(Address_ptr(new IPv6Address()), 0u));
        }
        return true;
    }

    std::multimap<std::string, std::pair<Address_ptr, uint32_t> > results;

    if (!GetInterfaceAddresses(results, family)) {
        return false;
    }

    auto its = results.equal_range(iface);
    for (; its.first != its.second; ++its.first) {
        result.push_back(its.first->second);
    }
    return !result.empty();
}

Address::~Address() {}

int Address::getFamily() const {
    return getAddr()->sa_family;
}

std::string Address::toString() const {
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

bool Address::operator<(const Address& rhs) const {
    socklen_t minlen = std::min(getAddrLen(), rhs.getAddrLen());
    int result = memcmp(getAddr(), rhs.getAddr(), minlen);
    if (result < 0) return true;
    else if (result > 0) return false;
    else if (getAddrLen() < rhs.getAddrLen()) return true;
    else return false;
}

bool Address::operator==(const Address& rhs) const {
    return getAddrLen() == rhs.getAddrLen()
        && memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
}

//****************************************************************************
// IPAddress
//****************************************************************************

IPAddress_ptr IPAddress::Create(const char* address, uint16_t port) {
    addrinfo hints, * results;
    memset(&hints, 0, sizeof(addrinfo));

    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;

    int error = getaddrinfo(address, "http", &hints, &results);
    if (error) {
        SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) 
            << "IPAddress::Create(" << address
            << ", " << port << ") error=" << error
            << " errno=" << errno << " errstr=" << strerror(errno);
        return nullptr;
    }

    try {
        IPAddress_ptr result = std::dynamic_pointer_cast<IPAddress>(
            Address::Create(results->ai_addr, (socklen_t)results->ai_addrlen));
        if (result) {
            result->setPort(port);
        }
        freeaddrinfo(results);
        return result;
    }
    catch (...) {
        freeaddrinfo(results);
        return nullptr;
    }
}

//****************************************************************************
// IPv4Address
//****************************************************************************

IPv4Address_ptr IPv4Address::Create(const char* address, uint16_t port) {
    IPv4Address_ptr rt(new IPv4Address);
    rt->__address.sin_port = byteswapOnLittleEndian(port);
    int result = inet_pton(AF_INET, address, &rt->__address.sin_addr.s_addr);
    if (result <= 0) {
        SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) 
            << "IPv4Address::Create(" << address << ", "
            << port << ") rt=" << result << " errno=" << errno
            << " errstr=" << strerror(errno);
        return nullptr;
    }
    return rt;
}

IPv4Address::IPv4Address(const sockaddr_in& address) {
    __address = address;
}

IPv4Address::IPv4Address(uint32_t address, uint16_t port) {
    memset(&__address, 0, sizeof(__address));
    __address.sin_family = AF_INET;
    __address.sin_port = byteswapOnLittleEndian(port);
    __address.sin_addr.s_addr = byteswapOnLittleEndian(address);
}

const sockaddr* IPv4Address::getAddr() const {
    return (sockaddr*)&__address;
}

sockaddr* IPv4Address::getAddr() {
    return (sockaddr*)&__address;
}

socklen_t IPv4Address::getAddrLen() const {
    return sizeof(__address);
}

std::ostream& IPv4Address::insert(std::ostream& os) const {
    uint32_t addr = byteswapOnLittleEndian(__address.sin_addr.s_addr);
    os << ((addr >> 24) & 0xff) << "."
        << ((addr >> 16) & 0xff) << "."
        << ((addr >> 8) & 0xff) << "."
        << (addr & 0xff);
    os << ":" << byteswapOnLittleEndian(__address.sin_port);
    return os;
}

IPAddress_ptr IPv4Address::broadcastAddress(uint32_t prefix_len) {
    if (prefix_len > 32) {
        return nullptr;
    }

    sockaddr_in baddr(__address);
    baddr.sin_addr.s_addr |= ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address_ptr(new IPv4Address(baddr));
}

IPAddress_ptr IPv4Address::networkAddress(uint32_t prefix_len) {
    if (prefix_len > 32) {
        return nullptr;
    }

    sockaddr_in baddr(__address);
    baddr.sin_addr.s_addr &= byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address_ptr(new IPv4Address(baddr));
}

IPAddress_ptr IPv4Address::subnetMask(uint32_t prefix_len) {
    sockaddr_in subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin_family = AF_INET;
    subnet.sin_addr.s_addr = byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address_ptr(new IPv4Address(subnet));
}

uint32_t IPv4Address::IPv4Address::getPort() const {
    return byteswapOnLittleEndian(__address.sin_port);
}

void IPv4Address::setPort(uint16_t v) {
    __address.sin_port = byteswapOnLittleEndian(v);
}

//****************************************************************************
// IPv6Address
//****************************************************************************

IPv6Address_ptr IPv6Address::Create(const char* address, uint16_t port) {
    IPv6Address_ptr rt(new IPv6Address);
    rt->__address.sin6_port = byteswapOnLittleEndian(port);
    int result = inet_pton(AF_INET6, address, &rt->__address.sin6_addr.__in6_u.__u6_addr8);
    if (result <= 0) {
        SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) 
            << "IPv6Address::Create(" << address << ", "
            << port << ") rt=" << result << " errno=" << errno
            << " errstr=" << strerror(errno);
        return nullptr;
    }
    return rt;
}

IPv6Address::IPv6Address() {
    memset(&__address, 0, sizeof(__address));
    __address.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6& address) {
    __address = address;
}

IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port) {
    memset(&__address, 0, sizeof(__address));
    __address.sin6_family = AF_INET6;
    __address.sin6_port = byteswapOnLittleEndian(port);
    memcpy(&__address.sin6_addr.__in6_u.__u6_addr8, address, 16);
}

const sockaddr* IPv6Address::getAddr() const {
    return (sockaddr*)&__address;
}

sockaddr* IPv6Address::getAddr() {
    return (sockaddr*)&__address;
}

socklen_t IPv6Address::getAddrLen() const {
    return sizeof(__address);
}

std::ostream& IPv6Address::insert(std::ostream& os) const {
    os << "[";
    uint16_t* addr = (uint16_t*)__address.sin6_addr.s6_addr;
    bool used_zeros = false;
    for (size_t i = 0; i < 8; ++i) {
        if (addr[i] == 0 && !used_zeros) {
            continue;
        }
        if (i && addr[i - 1] == 0 && !used_zeros) {
            os << ":";
            used_zeros = true;
        }
        if (i) {
            os << ":";
        }
        os << std::hex << (int)byteswapOnLittleEndian(addr[i]) << std::dec;
    }

    if (!used_zeros && addr[7] == 0) {
        os << "::";
    }

    os << "]:" << byteswapOnLittleEndian(__address.sin6_port);
    return os;
}

IPAddress_ptr IPv6Address::broadcastAddress(uint32_t prefix_len) {
    sockaddr_in6 baddr(__address);
    baddr.sin6_addr.__in6_u.__u6_addr8[prefix_len / 8] |= ~CreateMask<uint8_t>(prefix_len % 8);
    for (int i = prefix_len / 8 + 1; i < 16; ++i) {
        baddr.sin6_addr.__in6_u.__u6_addr8[i] = 0xff;
    }
    return IPv6Address_ptr(new IPv6Address(baddr));
}

IPAddress_ptr IPv6Address::networkAddress(uint32_t prefix_len) {
    sockaddr_in6 baddr(__address);
    baddr.sin6_addr.__in6_u.__u6_addr8[prefix_len / 8] &= CreateMask<uint8_t>(prefix_len % 8);
    for (int i = prefix_len / 8 + 1; i < 16; ++i) {
        baddr.sin6_addr.__in6_u.__u6_addr8[i] = 0x00;
    }
    return IPv6Address_ptr(new IPv6Address(baddr));
}

IPAddress_ptr IPv6Address::subnetMask(uint32_t prefix_len) {
    sockaddr_in6 subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin6_family = AF_INET6;
    subnet.sin6_addr.__in6_u.__u6_addr8[prefix_len / 8] = CreateMask<uint8_t>(prefix_len % 8);

    for (uint32_t i = 0; i < prefix_len / 8; ++i) {
        subnet.sin6_addr.__in6_u.__u6_addr8[i] = 0xff;
    }
    return IPv6Address_ptr(new IPv6Address(subnet));
}

uint32_t IPv6Address::getPort() const {
    return byteswapOnLittleEndian(__address.sin6_port);
}

void IPv6Address::setPort(uint16_t v) {
    __address.sin6_port = byteswapOnLittleEndian(v);
}

//****************************************************************************
// UnixAddress
//****************************************************************************

static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0)->sun_path) - 1;

UnixAddress::UnixAddress() {
    memset(&__address, 0, sizeof(__address));
    __address.sun_family = AF_UNIX;
    __length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

UnixAddress::UnixAddress(const std::string& path) {
    memset(&__address, 0, sizeof(__address));
    __address.sun_family = AF_UNIX;
    __length = path.size() + 1;

    if (!path.empty() && path[0] == '\0') {
        --__length;
    }

    if (__length > sizeof(__address.sun_path)) {
        throw std::logic_error("path too long");
    }
    memcpy(__address.sun_path, path.c_str(), __length);
    __length += offsetof(sockaddr_un, sun_path);
}

const sockaddr* UnixAddress::getAddr() const {
    return (sockaddr*)&__address;
}

sockaddr* UnixAddress::getAddr() {
    return (sockaddr*)&__address;
}

socklen_t UnixAddress::getAddrLen() const {
    return __length;
}

void UnixAddress::setAddrLen(uint32_t v) {
    __length = v;
}

std::string UnixAddress::getPath() const {
    std::stringstream ss;
    if (__length > offsetof(sockaddr_un, sun_path)
        && __address.sun_path[0] == '\0') {
        ss << "\\0" << std::string(__address.sun_path + 1,
                                   __length - offsetof(sockaddr_un, sun_path) - 1);
    }
    else {
        ss << __address.sun_path;
    }
    return ss.str();
}

std::ostream& UnixAddress::insert(std::ostream& os) const {
    if (__length > offsetof(sockaddr_un, sun_path)
        && __address.sun_path[0] == '\0') {
        return os << "\\0" << std::string(__address.sun_path + 1, __length - offsetof(sockaddr_un, sun_path) - 1);
    }
    return os << __address.sun_path;
}

//****************************************************************************
// UnknownAddress
//****************************************************************************

UnknownAddress::UnknownAddress(int family) {
    memset(&__address, 0, sizeof(__address));
    __address.sa_family = family;
}

UnknownAddress::UnknownAddress(const sockaddr& addr) {
    __address = addr;
}

const sockaddr* UnknownAddress::getAddr() const {
    return (sockaddr*)&__address;
}

sockaddr* UnknownAddress::getAddr() {
    return &__address;
}

socklen_t UnknownAddress::getAddrLen() const {
    return sizeof(__address);
}

std::ostream& UnknownAddress::insert(std::ostream& os) const {
    os << "[UnknownAddress family=" << __address.sa_family << "]";
    return os;
}


}; /* sylar */