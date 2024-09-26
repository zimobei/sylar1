//*****************************************************************************
//
//
//   ��ͷ�ļ�ʵ�� TCP �������ķ�װ
//  
//
//*****************************************************************************

#ifndef SYLAR_TCP_SERVER_H
#define SYLAR_TCP_SERVER_H

#include <sstream>
#include <memory>
#include <functional>
#include <vector>
#include <string>
#include <map>
#include <yaml-cpp/yaml.h>
#include <boost/noncopyable.hpp>
#include "LexicalCast.h"
#include "IOManager.h"
#include "Address.h"
#include "Socket.h"
#include "Config.h"

namespace sylar
{

//****************************************************************************
// ǰ������
//****************************************************************************

struct TcpServerConf;
using TcpServerConf_ptr = std::shared_ptr<TcpServerConf>;

class TcpServer;
using TcpServer_ptr = std::shared_ptr<TcpServer>;

//****************************************************************************
// TCP ���ýṹ
//****************************************************************************

struct TcpServerConf {
	std::vector<std::string> address;
	int keepalive = 0;
	int timeout = 1000 * 2 * 60;
	int ssl = 0;
	std::string id;
	std::string type = "http";                  // ���������ͣ�http, ws, rock
	std::string name;
	std::string cert_file;
	std::string key_file;
	std::string accept_worker;
	std::string io_worker;
	std::string process_worker;
	std::map<std::string, std::string> args;

	bool isValid() const;

	bool operator==(const TcpServerConf& oth) const;
};

//****************************************************************************
// �Զ�������ת��
//****************************************************************************

template<>
class LexicalCast<std::string, TcpServerConf> {
public:
	TcpServerConf operator()(const std::string& v) {
		YAML::Node node = YAML::Load(v);
		TcpServerConf conf;
		conf.id = node["id"].as<std::string>(conf.id);
		conf.type = node["type"].as<std::string>(conf.type);
		conf.keepalive = node["keepalive"].as<int>(conf.keepalive);
		conf.timeout = node["timeout"].as<int>(conf.timeout);
		conf.name = node["name"].as<std::string>(conf.name);
		conf.ssl = node["ssl"].as<int>(conf.ssl);
		conf.cert_file = node["cert_file"].as<std::string>(conf.cert_file);
		conf.key_file = node["key_file"].as<std::string>(conf.key_file);
		conf.accept_worker = node["accept_worker"].as<std::string>();
		conf.io_worker = node["io_worker"].as<std::string>();
		conf.process_worker = node["process_worker"].as<std::string>();
		conf.args = LexicalCast<std::string, std::map<std::string, std::string> >()(node["args"].as<std::string>(""));
		if (node["address"].IsDefined()) {
			for (size_t i = 0; i < node["address"].size(); ++i) {
				conf.address.push_back(node["address"][i].as<std::string>());
			}
		}
		return conf;
	}
};

template<>
class LexicalCast<TcpServerConf, std::string> {
public:
	std::string operator()(const TcpServerConf& conf) {
		YAML::Node node;
		node["id"] = conf.id;
		node["type"] = conf.type;
		node["name"] = conf.name;
		node["keepalive"] = conf.keepalive;
		node["timeout"] = conf.timeout;
		node["ssl"] = conf.ssl;
		node["cert_file"] = conf.cert_file;
		node["key_file"] = conf.key_file;
		node["accept_worker"] = conf.accept_worker;
		node["io_worker"] = conf.io_worker;
		node["process_worker"] = conf.process_worker;
		node["args"] = YAML::Load(LexicalCast<std::map<std::string, std::string>, std::string>()(conf.args));
		for (auto& i : conf.address) {
			node["address"].push_back(i);
		}
		std::stringstream ss;
		ss << node;
		return ss.str();
	}
};

//****************************************************************************
// TCP��������װ
//****************************************************************************

class TcpServer : public std::enable_shared_from_this<TcpServer>, boost::noncopyable {
protected:
	std::vector<Socket_ptr> __socks;        // ����Socket����
	IOManager* __worker;                    // �����ӵ�Socket�����ĵ�����
	IOManager* __ioWorker;
	IOManager* __acceptWorker;              // ������Socket�������ӵĵ�����
	uint64_t __recvTimeout;                 // ���ճ�ʱʱ��(����)
	std::string __name;                     // ����������
	std::string __type = "tcp";             // ����������
	bool __isStop;                          // �����Ƿ�ֹͣ
	bool __ssl = false;
	TcpServerConf_ptr __conf;
protected:
	/*!
	 * @brief ���������ӵ�Socket��
	 */
	virtual void handleClient(Socket_ptr client);

	/*!
	 * @brief ��ʼ��������
	 */
	virtual void startAccept(Socket_ptr sock);
public:
	/*!
	 * @brief ���캯��
	 * @param worker socket�ͻ��˹�����Э�̵�����
	 * @param accept_worker ������socketִ�н���socket���ӵ�Э�̵�����
	 */
	TcpServer(IOManager* worker = IOManager::GetThis(),
			  IOManager* io_worker = IOManager::GetThis(),
			  IOManager* accept_worker = IOManager::GetThis());

	/*!
	 * @brief ��������
	 */
	virtual ~TcpServer();

	/*!
	 * @brief �󶨵�ַ
	 * @return �����Ƿ�󶨳ɹ�
	 */
	virtual bool bind(Address_ptr addr);

	/*!
	 * @brief �󶨵�ַ����
	 * @param addrs ��Ҫ�󶨵ĵ�ַ����
	 * @param fails ��ʧ�ܵĵ�ַ
	 * @return �Ƿ�󶨳ɹ�
	 */
	virtual bool bind(const std::vector<Address_ptr>& addrs,
			  std::vector<Address_ptr>& fails);

	bool loadCertificates(const std::string& cert_file, const std::string& key_file);

	/*!
	 * @brief ��������
	 * @return ��Ҫbind�ɹ���ִ��
	 */
	virtual bool start();

	/*!
	 * @brief ֹͣ����
	 */
	virtual void stop();

	/*!
	 * @brief ���ض�ȡ��ʱʱ��(����)
	 */
	uint64_t getRecvTimeout() const;

	/*!
	 * @brief ���ط���������
	 */
	std::string getName() const;

	/*!
	 * @brief ���ö�ȡ��ʱʱ��(����)
	 */
	void setRecvTimeout(uint64_t v);

	/*!
	 * @brief ���÷���������
	 */
	virtual void setName(const std::string& v);

	/*!
	 * @brief �Ƿ�ֹͣ
	 */
	bool isStop() const;

	 TcpServerConf_ptr getConf() const;
	 void setConf(TcpServerConf_ptr v);
	 void setConf(const TcpServerConf& v);

	virtual std::string toString(const std::string& prefix = "");
	std::vector<Socket_ptr> getSocks() const;
};

}; /* sylar */

#endif /* SYLAR_TCP_SERVER_H */
