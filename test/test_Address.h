#ifndef SYLAR_TEST_ADDRESS_H
#define SYLAR_TEST_ADDRESS_H

#include "Address.h"
#include "Log.h"
#include <iostream>

using std::cout;
using std::endl;
using namespace sylar;

namespace Test
{

// 通过静态方法能够通过域名、IP、主机名获得对应Address
void test_address_test() {
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "test_address_test";
    std::vector<sylar::Address_ptr> addrs;
    bool v = sylar::Address::Lookup(addrs, "www.baidu.com", AF_INET, SOCK_STREAM);

    if (!v) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "lookup fail";
    }

    for (size_t i = 0; i < addrs.size(); ++i) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << i << " - " << addrs[i]->toString();
    }
}

// 通过静态方法获得本机的Address
void test_address_iface() {
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "test_address_iface";
    std::multimap<std::string, std::pair<sylar::Address_ptr, uint32_t> > results;

    bool v = sylar::Address::GetInterfaceAddresses(results);
    if (!v) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "GetInterfaceAddresses fail";
        return;
    }

    for (auto& i : results) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) 
            << i.first << " - " 
            << i.second.first->toString() << " - "
            << i.second.second;
    }
}

// 通过静态方法获取IPAddress
void test_address_ip() {
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "test_address_ip";
    auto addr = sylar::IPAddress::Create("www.baidu.com");
    if (addr) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << addr->toString();
    }
}

// 创建 IPv4 地址
void test_address_ipv4() {
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "test_address_ipv4";
    auto addr = sylar::IPv4Address::Create("172.23.253.254", 80);
    auto saddr = addr->subnetMask(24);
    auto baddr = addr->broadcastAddress(24);
    auto naddr = addr->networkAddress(24);
    if (addr) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << addr->toString();
    }
    if (saddr) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << saddr->toString();
    }
    if (baddr) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << baddr->toString();
    }
    if (naddr) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << naddr->toString();
    }
}

// 创建 IPv6 地址
void test_address_ipv6() {
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "test_address_ipv6";
    auto addr = sylar::IPv6Address::Create("fe80::215:5dff:fec7:b3dd", 8020);
    if (addr) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << addr->toString();
    }
    auto saddr = addr->subnetMask(64);
    auto baddr = addr->broadcastAddress(64);
    auto naddr = addr->networkAddress(64);
    if (addr) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << addr->toString();
    }
    if (saddr) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << saddr->toString();
    }
    if (baddr) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << baddr->toString();
    }
    if (naddr) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << naddr->toString();
    }
}

void test_address() {
    cout << "-------------------------------- test address ----------------------------" << endl;
    test_address_test();
    test_address_iface();
    test_address_ip();
    test_address_ipv4();
    test_address_ipv6();
    cout << "-------------------------------- test over ----------------------------" << endl;
}

};

#endif /* SYLAR_TEST_ADDRESS_H */
