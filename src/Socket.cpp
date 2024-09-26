#include "Socket.h"
#include "IOManager.h"
#include "FDManager.h"
#include "Log.h"
#include "Macro.h"
#include "Hook.h"
#include "Util.h"

namespace sylar
{

//****************************************************************************
// 流式处理 socket
//****************************************************************************

std::ostream& operator<<(std::ostream& os, const Socket& sock) {
	return sock.dump(os);
}

//****************************************************************************
// Socket
//****************************************************************************

void Socket::initSock() {
	int val = 1;
	setOption(SOL_SOCKET, SO_REUSEADDR, val);
	if (__type == SOCK_STREAM) {
		setOption(IPPROTO_TCP, TCP_NODELAY, val);
	}
}

void Socket::newSock() {
	__sock = socket(__family, __type, __protocol);
	if (SYLAR_LIKELY(__sock != -1)) initSock();
	else {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
			<< "socket(" << __family 
			<< ", " << __type 
			<< ", " << __protocol 
			<< ") errno=" << errno 
			<< " errstr=" << strerror(errno);
	}
}

bool Socket::init(int sock) {
	FDCtx_ptr ctx = FDManager_single::GetInstance()->get(sock);
	if (ctx && ctx->isSocket() && !ctx->isClose()) {
		__sock = sock;
		__isConnected = true;
		initSock();
		getLocalAddress();
		getRemoteAddress();
		return true;
	}
	return false;
}

Socket_ptr Socket::CreateTCP(Address_ptr address) {
	Socket_ptr result(new Socket(address->getFamily(), TCP, 0));
	return result;
}

Socket_ptr Socket::CreateUDP(Address_ptr address) {
	Socket_ptr result(new Socket(address->getFamily(), UDP, 0));
	return result;
}

Socket_ptr Socket::CreateTCPSocket() {
	Socket_ptr result(new Socket(IPv4, TCP, 0));
	return result;
}

Socket_ptr Socket::CreateUDPSocket() {
	Socket_ptr result(new Socket(IPv4, UDP, 0));
	return result;
}

Socket_ptr Socket::CreateTCPSocket6() {
	Socket_ptr result(new Socket(IPv6, TCP, 0));
	return result;
}

Socket_ptr Socket::CreateUDPSocket6() {
	Socket_ptr result(new Socket(IPv6, UDP, 0));
	return result;
}

Socket_ptr Socket::CreateUnixTCPSocket() {
	Socket_ptr result(new Socket(UNIX, TCP, 0));
	return result;
}

Socket_ptr Socket::CreateUnixUDPSocket() {
	Socket_ptr result(new Socket(UNIX, UDP, 0));
	return result;
}

Socket::Socket(int family, int type, int protocol)
	: __sock(-1),
	__family(family),
	__type(type),
	__protocol(protocol),
	__isConnected(false){}

Socket::~Socket() {
	close();
}

int64_t Socket::getSendTimeout() {
	FDCtx_ptr ctx = FDManager_single::GetInstance()->get(__sock);
	if (ctx) return ctx->getTimeout(SO_SNDTIMEO);
	else return -1;
}

void Socket::setSendTimeout(int64_t v) {
	struct timeval tv { int(v / 1000), int(v % 1000 * 1000) };
	setOption(SOL_SOCKET, SO_SNDTIMEO, tv);
}

int64_t Socket::getRecvTimeout() {
	FDCtx_ptr ctx = FDManager_single::GetInstance()->get(__sock);
	if (ctx) return ctx->getTimeout(SO_RCVTIMEO);
	else return -1;
}

void Socket::setRecvTimeout(int64_t v) {
	struct timeval tv { int(v / 1000), int(v % 1000 * 1000) };
	setOption(SOL_SOCKET, SO_RCVTIMEO, tv);
}

bool Socket::getOption(int level, int option, void* result, socklen_t* len) {
	int rt = getsockopt(__sock, level, option, result, (socklen_t*)len);
	if (rt) {
		SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) 
			<< "getOption sock=" << __sock
			<< " level=" << level 
			<< " option=" << option
			<< " errno=" << errno 
			<< " errstr=" << strerror(errno);
		return false;
	}
	return true;
}

bool Socket::setOption(int level, int option, const void* result, socklen_t len) {
	if (setsockopt(__sock, level, option, result, (socklen_t)len)) {
		SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) 
			<< "setOption sock=" << __sock
			<< " level=" << level 
			<< " option=" << option
			<< " errno=" << errno 
			<< " errstr=" << strerror(errno);
		return false;
	}
	return true;
}

Socket_ptr Socket::accept() {
	Socket_ptr sock(new Socket(__family, __type, __protocol));
	int newsock = ::accept(__sock, nullptr, nullptr);
	if (newsock == -1) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
			<< "accept(" << __sock 
			<< ") errno=" << errno 
			<< " errstr=" << strerror(errno);
		return nullptr;
	}
	if (sock->init(newsock)) {
		return sock;
	}
	return nullptr;
}

bool Socket::bind(const Address_ptr addr) {
	if (!isValid()) {
		newSock();
		if (SYLAR_UNLIKELY(!isValid())) return false;
	}

	if (SYLAR_UNLIKELY(addr->getFamily() != __family)) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
			<< "bind sock.family("<< __family 
			<< ") addr.family(" << addr->getFamily()
			<< ") not equal, addr=" << addr->toString();
		return false;
	}

	UnixAddress_ptr uaddr = std::dynamic_pointer_cast<UnixAddress>(addr);
	if (uaddr) {
		Socket_ptr sock = Socket::CreateUnixTCPSocket();
		if (sock->connect(uaddr)) {
			return false;
		}
		else {
			FSUtil::Unlink(uaddr->getPath(), true);
		}
	}

	if (::bind(__sock, addr->getAddr(), addr->getAddrLen())) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
			<< "bind error errrno=" << errno
			<< " errstr=" << strerror(errno);
		return false;
	}
	getLocalAddress();
	return true;
}

bool Socket::connect(const Address_ptr addr, uint64_t timeout_ms) {
	__remoteAddress = addr;
	if (!isValid()) {
		newSock();
		if (SYLAR_UNLIKELY(!isValid())) {
			return false;
		}
	}

	if (SYLAR_UNLIKELY(addr->getFamily() != __family)) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
			<< "connect sock.family(" << __family 
			<< ") addr.family(" << addr->getFamily()
			<< ") not equal, addr=" << addr->toString();
		return false;
	}

	if (timeout_ms == (uint64_t)-1) {
		if (::connect(__sock, addr->getAddr(), addr->getAddrLen())) {
			SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
				<< "sock=" << __sock 
				<< " connect(" << addr->toString()
				<< ") error errno=" << errno 
				<< " errstr=" << strerror(errno);
			close();
			return false;
		}
	}
	else {
		if (::connect_with_timeout(__sock, addr->getAddr(), addr->getAddrLen(), timeout_ms)) {
			SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
				<< "sock=" << __sock 
				<< " connect(" << addr->toString()
				<< ") timeout=" << timeout_ms 
				<< " error errno=" << errno 
				<< " errstr=" << strerror(errno);
			close();
			return false;
		}
	}
	__isConnected = true;
	getRemoteAddress();
	getLocalAddress();
	return true;
}

bool Socket::reconnect(uint64_t timeout_ms) {
	if (!__remoteAddress) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "reconnect __remoteAddress is null";
		return false;
	}
	__localAddress.reset();
	return connect(__remoteAddress, timeout_ms);
}

bool Socket::listen(int backlog) {
	if (!isValid()) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "listen error sock=-1";
		return false;
	}
	if (::listen(__sock, backlog)) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
			<< "listen error errno=" << errno
			<< " errstr=" << strerror(errno);
		return false;
	}
	return true;
}

bool Socket::close() {
	if (!__isConnected && __sock == -1) {
		return true;
	}
	__isConnected = false;
	if (__sock != -1) {
		::close(__sock);
		__sock = -1;
	}
	return true;
}

int Socket::send(const void* buffer, size_t length, int flags) {
	if (isConnected()) {
		return ::send(__sock, buffer, length, flags);
	}
	return -1;
}

int Socket::send(const iovec* buffers, size_t length, int flags) {
	if (isConnected()) {
		msghdr msg;
		memset(&msg, 0, sizeof(msg));
		msg.msg_iov = (iovec*)buffers;
		msg.msg_iovlen = length;
		return ::sendmsg(__sock, &msg, flags);
	}
	return -1;
}

int Socket::sendTo(const void* buffer, size_t length, const Address_ptr to, int flags) {
	if (isConnected()) {
		return ::sendto(__sock, buffer, length, flags, to->getAddr(), to->getAddrLen());
	}
	return -1;
}

int Socket::sendTo(const iovec* buffers, size_t length, const Address_ptr to, int flags) {
	if (isConnected()) {
		msghdr msg;
		memset(&msg, 0, sizeof(msg));
		msg.msg_iov = (iovec*)buffers;
		msg.msg_iovlen = length;
		msg.msg_name = to->getAddr();
		msg.msg_namelen = to->getAddrLen();
		return ::sendmsg(__sock, &msg, flags);
	}
	return -1;
}

int Socket::recv(void* buffer, size_t length, int flags) {
	if (isConnected()) {
		return ::recv(__sock, buffer, length, flags);
	}
	return -1;
}

int Socket::recv(iovec* buffers, size_t length, int flags) {
	if (isConnected()) {
		msghdr msg;
		memset(&msg, 0, sizeof(msg));
		msg.msg_iov = (iovec*)buffers;
		msg.msg_iovlen = length;
		return ::recvmsg(__sock, &msg, flags);
	}
	return -1;
}

int Socket::recvFrom(void* buffer, size_t length, Address_ptr from, int flags) {
	if (isConnected()) {
		socklen_t len = from->getAddrLen();
		return ::recvfrom(__sock, buffer, length, flags, from->getAddr(), &len);
	}
	return -1;
}

int Socket::recvFrom(iovec* buffers, size_t length, Address_ptr from, int flags) {
	if (isConnected()) {
		msghdr msg;
		memset(&msg, 0, sizeof(msg));
		msg.msg_iov = (iovec*)buffers;
		msg.msg_iovlen = length;
		msg.msg_name = from->getAddr();
		msg.msg_namelen = from->getAddrLen();
		return ::recvmsg(__sock, &msg, flags);
	}
	return -1;
}

Address_ptr Socket::getRemoteAddress() {
	if (__remoteAddress) {
		return __remoteAddress;
	}

	Address_ptr result;
	switch (__family) {
		case AF_INET:
			result.reset(new IPv4Address());
			break;
		case AF_INET6:
			result.reset(new IPv6Address());
			break;
		case AF_UNIX:
			result.reset(new UnixAddress());
			break;
		default:
			result.reset(new UnknownAddress(__family));
			break;
	}
	socklen_t addrlen = result->getAddrLen();
	if (getpeername(__sock, result->getAddr(), &addrlen)) {
		return Address_ptr(new UnknownAddress(__family));
	}
	if (__family == AF_UNIX) {
		UnixAddress_ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
		addr->setAddrLen(addrlen);
	}
	__remoteAddress = result;
	return __remoteAddress;
}

Address_ptr Socket::getLocalAddress() {
	if (__localAddress) {
		return __localAddress;
	}

	Address_ptr result;
	switch (__family) {
		case AF_INET:
			result.reset(new IPv4Address());
			break;
		case AF_INET6:
			result.reset(new IPv6Address());
			break;
		case AF_UNIX:
			result.reset(new UnixAddress());
			break;
		default:
			result.reset(new UnknownAddress(__family));
			break;
	}
	socklen_t addrlen = result->getAddrLen();
	if (getsockname(__sock, result->getAddr(), &addrlen)) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
			<< "getsockname error sock=" << __sock
			<< " errno=" << errno 
			<< " errstr=" << strerror(errno);
		return Address_ptr(new UnknownAddress(__family));
	}
	if (__family == AF_UNIX) {
		UnixAddress_ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
		addr->setAddrLen(addrlen);
	}
	__localAddress = result;
	return __localAddress;
}

int Socket::getFamily() const {
	return __family;
}

int Socket::getType() const {
	return __type;
}

int Socket::getProtocol() const {
	return __protocol;
}

bool Socket::isConnected() const {
	return __isConnected;
}

bool Socket::isValid() const {
	return __sock != -1;
}

int Socket::getError() {
	int error = 0;
	socklen_t len = sizeof(error);
	if (!getOption(SOL_SOCKET, SO_ERROR, &error, &len)) {
		error = errno;
	}
	return error;
}

std::ostream& Socket::dump(std::ostream& os) const {
	os  << "[Socket sock=" << __sock
		<< " is_connected=" << __isConnected
		<< " family=" << __family
		<< " type=" << __type
		<< " protocol=" << __protocol;
	if (__localAddress) {
		os << " local_address=" << __localAddress->toString();
	}
	if (__remoteAddress) {
		os << " remote_address=" << __remoteAddress->toString();
	}
	os << "]";
	return os;
}

std::string Socket::toString() const {
	std::stringstream ss;
	dump(ss);
	return ss.str();
}

int Socket::getSocket() const {
	return __sock;
}

bool Socket::cancelRead() {
	return IOManager::GetThis()->cancelEvent(__sock, IOManager::READ);
}

bool Socket::cancelWrite() {
	return IOManager::GetThis()->cancelEvent(__sock, IOManager::WRITE);
}

bool Socket::cancelAccept() {
	return IOManager::GetThis()->cancelEvent(__sock, IOManager::READ);
}

bool Socket::cancelAll() {
	return IOManager::GetThis()->cancelAll(__sock);
}

//****************************************************************************
// SSLSocket
//****************************************************************************

namespace
{

struct _SSLInit {
	_SSLInit() {
		SSL_library_init();
		SSL_load_error_strings();
		OpenSSL_add_all_algorithms();
	}
};

static _SSLInit s_init;

}

bool SSLSocket::init(int sock) {
	bool v = Socket::init(sock);
	if (v) {
		__ssl.reset(SSL_new(__ctx.get()), SSL_free);
		SSL_set_fd(__ssl.get(), __sock);
		v = (SSL_accept(__ssl.get()) == 1);
	}
	return v;
}

SSLSocket_ptr SSLSocket::CreateTCP(Address_ptr address) {
	SSLSocket_ptr sock(new SSLSocket(address->getFamily(), TCP, 0));
	return sock;
}

SSLSocket_ptr SSLSocket::CreateTCPSocket() {
	SSLSocket_ptr sock(new SSLSocket(IPv4, TCP, 0));
	return sock;
}

SSLSocket_ptr SSLSocket::CreateTCPSocket6() {
	SSLSocket_ptr sock(new SSLSocket(IPv6, TCP, 0));
	return sock;
}

SSLSocket::SSLSocket(int family, int type, int protocol)
	: Socket(family, type, protocol){}

Socket_ptr SSLSocket::accept() {
	SSLSocket_ptr sock(new SSLSocket(__family, __type, __protocol));
	int newsock = ::accept(__sock, nullptr, nullptr);
	if (newsock == -1) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
			<< "accept(" << __sock 
			<< ") errno=" << errno 
			<< " errstr=" << strerror(errno);
		return nullptr;
	}
	sock->__ctx = __ctx;
	if (sock->init(newsock)) {
		return sock;
	}
	return nullptr;
}

bool SSLSocket::bind(const Address_ptr addr) {
	return Socket::bind(addr);
}

bool SSLSocket::connect(const Address_ptr addr, uint64_t timeout_ms) {
	bool v = Socket::connect(addr, timeout_ms);
	if (v) {
		__ctx.reset(SSL_CTX_new(SSLv23_client_method()), SSL_CTX_free);
		__ssl.reset(SSL_new(__ctx.get()), SSL_free);
		SSL_set_fd(__ssl.get(), __sock);
		v = (SSL_connect(__ssl.get()) == 1);
	}
	return v;
}

bool SSLSocket::listen(int backlog) {
	return Socket::listen(backlog);
}

bool SSLSocket::close() {
	return Socket::close();
}

int SSLSocket::send(const void* buffer, size_t length, int flags) {
	if (__ssl) {
		return SSL_write(__ssl.get(), buffer, length);
	}
	return -1;
}

int SSLSocket::send(const iovec* buffers, size_t length, int flags) {
	if (!__ssl) {
		return -1;
	}
	int total = 0;
	for (size_t i = 0; i < length; ++i) {
		int tmp = SSL_write(__ssl.get(), buffers[i].iov_base, buffers[i].iov_len);
		if (tmp <= 0) {
			return tmp;
		}
		total += tmp;
		if (tmp != (int)buffers[i].iov_len) {
			break;
		}
	}
	return total;
}

int SSLSocket::sendTo(const void* buffer, size_t length, const Address_ptr to, int flags) {
	SYLAR_ASSERT(false);
	return -1;
}

int SSLSocket::sendTo(const iovec* buffers, size_t length, const Address_ptr to, int flags) {
	SYLAR_ASSERT(false);
	return -1;
}

int SSLSocket::recv(void* buffer, size_t length, int flags) {
	if (__ssl) {
		return SSL_read(__ssl.get(), buffer, length);
	}
	return -1;
}

int SSLSocket::recv(iovec* buffers, size_t length, int flags) {
	if (!__ssl) {
		return -1;
	}
	int total = 0;
	for (size_t i = 0; i < length; ++i) {
		int tmp = SSL_read(__ssl.get(), buffers[i].iov_base, buffers[i].iov_len);
		if (tmp <= 0) {
			return tmp;
		}
		total += tmp;
		if (tmp != (int)buffers[i].iov_len) {
			break;
		}
	}
	return total;
}

int SSLSocket::recvFrom(void* buffer, size_t length, Address_ptr from, int flags) {
	SYLAR_ASSERT(false);
	return -1;
}

int SSLSocket::recvFrom(iovec* buffers, size_t length, Address_ptr from, int flags) {
	SYLAR_ASSERT(false);
	return -1;
}

bool SSLSocket::loadCertificates(const std::string& cert_file, const std::string& key_file) {
	__ctx.reset(SSL_CTX_new(SSLv23_server_method()), SSL_CTX_free);
	if (SSL_CTX_use_certificate_chain_file(__ctx.get(), cert_file.c_str()) != 1) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
			<< "SSL_CTX_use_certificate_chain_file("
			<< cert_file << ") error";
		return false;
	}
	if (SSL_CTX_use_PrivateKey_file(__ctx.get(), key_file.c_str(), SSL_FILETYPE_PEM) != 1) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
			<< "SSL_CTX_use_PrivateKey_file("
			<< key_file << ") error";
		return false;
	}
	if (SSL_CTX_check_private_key(__ctx.get()) != 1) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
			<< "SSL_CTX_check_private_key cert_file=" << cert_file 
			<< " key_file=" << key_file;
		return false;
	}
	return true;
}

std::ostream& SSLSocket::dump(std::ostream& os) const {
	os  << "[SSLSocket sock=" << __sock
		<< " is_connected=" << __isConnected
		<< " family=" << __family
		<< " type=" << __type
		<< " protocol=" << __protocol;
	if (__localAddress) {
		os << " local_address=" << __localAddress->toString();
	}
	if (__remoteAddress) {
		os << " remote_address=" << __remoteAddress->toString();
	}
	os << "]";
	return os;
}

}; /* sylar */