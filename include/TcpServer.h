//*****************************************************************************
//
//
//   此头文件实现 TCP 服务器的封装
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
// 前置声明
//****************************************************************************

struct TcpServerConf;
using TcpServerConf_ptr = std::shared_ptr<TcpServerConf>;

class TcpServer;
using TcpServer_ptr = std::shared_ptr<TcpServer>;

//****************************************************************************
// TCP 配置结构
//****************************************************************************

struct TcpServerConf {
	std::vector<std::string> address;
	int keepalive = 0;
	int timeout = 1000 * 2 * 60;
	int ssl = 0;
	std::string id;
	std::string type = "http";                  // 服务器类型，http, ws, rock
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
// 自定义类型转换
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
// TCP服务器封装
//****************************************************************************

class TcpServer : public std::enable_shared_from_this<TcpServer>, boost::noncopyable {
protected:
	std::vector<Socket_ptr> __socks;        // 监听Socket数组
	IOManager* __worker;                    // 新连接的Socket工作的调度器
	IOManager* __ioWorker;
	IOManager* __acceptWorker;              // 服务器Socket接收连接的调度器
	uint64_t __recvTimeout;                 // 接收超时时间(毫秒)
	std::string __name;                     // 服务器名称
	std::string __type = "tcp";             // 服务器类型
	bool __isStop;                          // 服务是否停止
	bool __ssl = false;
	TcpServerConf_ptr __conf;
protected:
	/*!
	 * @brief 处理新连接的Socket类
	 */
	virtual void handleClient(Socket_ptr client);

	/*!
	 * @brief 开始接受连接
	 */
	virtual void startAccept(Socket_ptr sock);
public:
	/*!
	 * @brief 构造函数
	 * @param worker socket客户端工作的协程调度器
	 * @param accept_worker 服务器socket执行接收socket连接的协程调度器
	 */
	TcpServer(IOManager* worker = IOManager::GetThis(),
			  IOManager* io_worker = IOManager::GetThis(),
			  IOManager* accept_worker = IOManager::GetThis());

	/*!
	 * @brief 析构函数
	 */
	virtual ~TcpServer();

	/*!
	 * @brief 绑定地址
	 * @return 返回是否绑定成功
	 */
	virtual bool bind(Address_ptr addr);

	/*!
	 * @brief 绑定地址数组
	 * @param addrs 需要绑定的地址数组
	 * @param fails 绑定失败的地址
	 * @return 是否绑定成功
	 */
	virtual bool bind(const std::vector<Address_ptr>& addrs,
			  std::vector<Address_ptr>& fails);

	bool loadCertificates(const std::string& cert_file, const std::string& key_file);

	/*!
	 * @brief 启动服务
	 * @return 需要bind成功后执行
	 */
	virtual bool start();

	/*!
	 * @brief 停止服务
	 */
	virtual void stop();

	/*!
	 * @brief 返回读取超时时间(毫秒)
	 */
	uint64_t getRecvTimeout() const;

	/*!
	 * @brief 返回服务器名称
	 */
	std::string getName() const;

	/*!
	 * @brief 设置读取超时时间(毫秒)
	 */
	void setRecvTimeout(uint64_t v);

	/*!
	 * @brief 设置服务器名称
	 */
	virtual void setName(const std::string& v);

	/*!
	 * @brief 是否停止
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
