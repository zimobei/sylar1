#include "TcpServer.h"
#include "Log.h"

namespace sylar
{

//****************************************************************************
// ‘”œÓ
//****************************************************************************

static ConfigVar_ptr<uint64_t> g_tcp_server_read_timeout =
	Config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2),
					  "tcp server read timeout");

//****************************************************************************
// TcpServerConf
//****************************************************************************

bool TcpServerConf::isValid() const {
	return !address.empty();
}

bool TcpServerConf::operator==(const TcpServerConf& oth) const {
	return address == oth.address
		&& keepalive == oth.keepalive
		&& timeout == oth.timeout
		&& name == oth.name
		&& ssl == oth.ssl
		&& cert_file == oth.cert_file
		&& key_file == oth.key_file
		&& accept_worker == oth.accept_worker
		&& io_worker == oth.io_worker
		&& process_worker == oth.process_worker
		&& args == oth.args
		&& id == oth.id
		&& type == oth.type;
}

//****************************************************************************
// TcpServer
//****************************************************************************

void TcpServer::handleClient(Socket_ptr client) {
	SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "handleClient: " << *client;
}

void TcpServer::startAccept(Socket_ptr sock) {
	while (!__isStop) {
		Socket_ptr client = sock->accept();
		if (client) {
			SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << "accept client" << client;
			client->setRecvTimeout(__recvTimeout);
			__worker->schedule(std::bind(&TcpServer::handleClient, shared_from_this(), client));
		}
		else {
			SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
				<< "accept errno=" << errno
				<< " errstr=" << strerror(errno);
		}
	}
}

TcpServer::TcpServer(IOManager* worker, IOManager* io_worker,
					 IOManager* accept_worker)
	:__worker(worker)
	, __acceptWorker(accept_worker)
	, __recvTimeout(g_tcp_server_read_timeout->getValue())
	, __name("sylar/1.0.0")
	, __isStop(true) {}

TcpServer::~TcpServer() {
	for (auto& i : __socks) {
		i->close();
	}
	__socks.clear();
}

bool TcpServer::bind(Address_ptr addr) {
	std::vector<Address_ptr> addrs;
	std::vector<Address_ptr> fails;
	addrs.push_back(addr);
	return bind(addrs, fails);
}

bool TcpServer::bind(const std::vector<Address_ptr>& addrs,
					 std::vector<Address_ptr>& fails) {
	for (auto& addr : addrs) {
		Socket_ptr sock = Socket::CreateTCP(addr);
		if (!sock->bind(addr)) {
			SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
				<< "bind fail errno=" << errno 
				<< " errstr=" << strerror(errno)
				<< " addr=[" << addr->toString() 
				<< "]";
			fails.push_back(addr);
			continue;
		}
		if (!sock->listen()) {
			SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) 
				<< "listen fail errno=" << errno 
				<< " errstr=" << strerror(errno)
				<< " addr=[" << addr->toString() 
				<< "]";
			fails.push_back(addr);
			continue;
		}
		__socks.push_back(sock);
	}

	if (!fails.empty()) {
		__socks.clear();
		return false;
	}

	for (auto& i : __socks) {
		SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) 
			<< " server bind success: " << *i;
	}
	return true;
}

bool TcpServer::loadCertificates(const std::string& cert_file,
								 const std::string& key_file) {
	for (auto& i : __socks) {
		auto ssl_socket = std::dynamic_pointer_cast<SSLSocket>(i);
		if (ssl_socket) {
			if (!ssl_socket->loadCertificates(cert_file, key_file)) {
				return false;
			}
		}
	}
	return true;
}

bool TcpServer::start() {
	if (!__isStop) {
		return true;
	}
	__isStop = false;
	for (auto& sock : __socks) {
		__acceptWorker->schedule(std::bind(&TcpServer::startAccept,
										   shared_from_this(), sock));
	}
	return true;
}

void TcpServer::stop() {
	__isStop = true;
	auto self = shared_from_this();
	__acceptWorker->schedule([this, self]() {
		for (auto& sock : __socks) {
			sock->cancelAll();
			sock->close();
		}
		__socks.clear();
	});
}

uint64_t TcpServer::getRecvTimeout() const {
	return __recvTimeout;
}

std::string TcpServer::getName() const {
	return __name;
}

void TcpServer::setRecvTimeout(uint64_t v) {
	__recvTimeout = v;
}

void TcpServer::setName(const std::string& v) {
	__name = v;
}

bool TcpServer::isStop() const {
	return __isStop;
}

TcpServerConf_ptr TcpServer::getConf() const {
	return __conf;
}

void TcpServer::setConf(TcpServerConf_ptr v) {
	__conf = v;
}

void TcpServer::setConf(const TcpServerConf& v) {
	__conf.reset(new TcpServerConf(v));
}

std::string TcpServer::toString(const std::string& prefix) {
	std::stringstream ss;
	ss  << prefix 
		<< "[ type=" << __type
		<< " name=" << __name 
		<< " ssl=" << __ssl
		<< " worker=" << (__worker ? __worker->getName() : "")
		<< " accept=" << (__acceptWorker ? __acceptWorker->getName() : "")
		<< " recv_timeout=" << __recvTimeout 
		<< " ]" << std::endl;
	std::string pfx = prefix.empty() ? "    " : prefix;
	for (auto& i : __socks) {
		ss << pfx << pfx << *i << std::endl;
	}
	return ss.str();
}

std::vector<Socket_ptr> TcpServer::getSocks() const {
	return __socks;
}

}; /* sylar */