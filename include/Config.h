//*****************************************************************************
//
//
//   此头文件实现配置系统功能
//  
//
//*****************************************************************************

#ifndef SYLAR_CONFIG_H
#define SYLAR_CONFIG_H

#include <string>
#include <map>
#include <list>
#include <memory>
#include <functional>
#include <yaml-cpp/yaml.h>

#include "Log.h"
#include "LexicalCast.h"
#include "Thread.h"

namespace sylar {

//****************************************************************************
// 前置声明
//****************************************************************************

class ConfigVarBase;
using ConfigVarBase_ptr = std::shared_ptr<ConfigVarBase>;

template<class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
class ConfigVar;

template<class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
using ConfigVar_ptr = std::shared_ptr<ConfigVar<T, FromStr, ToStr>>;

using ConfigVarMap = std::map<std::string, ConfigVarBase_ptr>;

//****************************************************************************
// 配置器
//****************************************************************************

/*!
 * @brief 配置变量的基类
 */
class ConfigVarBase {
protected:
	std::string __name;
	std::string __description;
public:
	ConfigVarBase(const std::string& name, const std::string& description = "");

	virtual ~ConfigVarBase();
public:
	const std::string& getName() const;
	const std::string& getDescription() const;

	/*!
	 * @brief 输出配置信息
	 */
	virtual std::string toString() = 0;

	/*!
	 * @brief 将配置文件中信息转化为 ConfigVarBase 对象信息
	 */
	virtual bool fromString(const std::string& val) = 0;

	/*!
	 * @brief 返回配置参数值的类型名称
	 */
	virtual std::string getTypeName() const = 0;
};

template<class T, class FromStr, class ToStr>
class ConfigVar : public ConfigVarBase {
public:
	using on_change_cb = typename std::function<void(const T& old_value, const T& new_value)>;
	using RWMutexType =  RWMutex;
private:
	T __val;
	std::map<uint64_t, on_change_cb> __cbs;
	RWMutexType __mutex;
public:
	
	ConfigVar(const std::string& name, const T& value, const std::string& description = "");

	/*!
	 * @brief 将参数值转换成YAML std::string
	 * @exception 当转换失败抛出异常 
	 */
	std::string toString() override;

	/*!
	 * @brief 从YAML std::string 转成参数的值
	 * @exception 当转换失败抛出异常
	 */
	bool fromString(const std::string& val) override;

	/*!
	 * @brief 返回 ConfigVar<T> 中 类型 T 的 name
	 */
	std::string getTypeName() const override;

	/*!
	 * @brief 设置值的时候，监听值是否发生，如果发生变化，做相应的操作
	 */
	void setValue(const T& val);

	/*!
	 * @brief 返回值
	 */
	const T& getValue();

	/*!
	 * @brief 增加监听
	 */
	uint64_t addListener(on_change_cb cb);

	/*!
	 * @brief 删除监听
	 */
	void delListener(uint64_t key);

	/*!
	 * @brief 获得监听器
	 */
	on_change_cb getListener(uint64_t key);

	/*!
	 * @brief 清空监听器
	 */
	void clearListener();
};

class Config {
public:
	using RWMutexType = RWMutex;
private:
	static ConfigVarMap& GetDatas();
	static RWMutexType& GetMutex();
public:
	template<class T>
	static ConfigVar_ptr<T> Lookup(const std::string& name, const T& value, const std::string& description = "");

	template<class T>
	static ConfigVar_ptr<T> Lookup(const std::string& name);

	static ConfigVarBase_ptr LookupBase(const std::string& name);

	static void LoadFromYaml(const YAML::Node& root);
};

//****************************************************************************
// 以下为模板类以及模板函数的实现
//****************************************************************************


//****************************************************************************
// ConfigVar<T, FromStr, ToStr>
//****************************************************************************

template<class T, class FromStr, class ToStr>
ConfigVar<T, FromStr, ToStr>::ConfigVar(const std::string& name, const T& value, const std::string& description)
	: ConfigVarBase(name, description), __val(value) {}

template<class T, class FromStr, class ToStr>
std::string ConfigVar<T, FromStr, ToStr>::toString() {
	try {
		RWMutexType::ReadLock lock(__mutex);
		return ToStr()(__val);
	} catch (std::exception& e) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
			<< "ConfigVar::tostd::string exception " << e.what()
			<< " convert: " << typeid(__val).name() << " to std::string"
			<< " name = " << __name;
	}
	return "";
}

template<class T, class FromStr, class ToStr>
bool ConfigVar<T, FromStr, ToStr>::fromString(const std::string& val) {
	try {
		setValue(FromStr()(val));
		return true;
	} catch (std::exception& e) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
			<< "ConfigVar::fromstd::string exception " << e.what()
			<< " convert: std::string to " << typeid(val).name()
			<< " - " << val;
	}
	return false;
}

template<class T, class FromStr, class ToStr>
std::string ConfigVar<T, FromStr, ToStr>::getTypeName() const {
	return typeid(T).name();
}

template<class T, class FromStr, class ToStr>
void ConfigVar<T, FromStr, ToStr>::setValue(const T& val) {
	{
		RWMutexType::ReadLock lock(__mutex);
		// 比较运算符需要在自定义类中重载
		if (val == __val) return;
		for (auto& i : __cbs) {
			// 依次执行回调函数，类似于观察者模式
			i.second(__val, val);
		}
	}
	RWMutexType::WriteLock lock(__mutex);
	// 赋值
	__val = val;
}

template<class T, class FromStr, class ToStr>
const T& ConfigVar<T, FromStr, ToStr>::getValue() {
	RWMutexType::ReadLock lock(__mutex);
	return __val;
}

template<class T, class FromStr, class ToStr>
uint64_t ConfigVar<T, FromStr, ToStr>::addListener(on_change_cb cb) {
	static uint64_t s_fun_id = 0;
	RWMutexType::WriteLock lock(__mutex);
	++s_fun_id;
	__cbs[s_fun_id] = cb;
	return s_fun_id;
}

template<class T, class FromStr, class ToStr>
void ConfigVar<T, FromStr, ToStr>::delListener(uint64_t key) {
	RWMutexType::WriteLock lock(__mutex);
	__cbs.erase(key);
}

template<class T, class FromStr, class ToStr>
typename ConfigVar<T, FromStr, ToStr>::on_change_cb
ConfigVar<T, FromStr, ToStr>::getListener(uint64_t key) {
	RWMutexType::ReadLock lock(__mutex);
	auto it = __cbs.find(key);
	return it == __cbs.end() ? nullptr : it->second;
}

template<class T, class FromStr, class ToStr>
void ConfigVar<T, FromStr, ToStr>::clearListener() {
	RWMutexType::WriteLock lock(__mutex);
	__cbs.clear();
}

//****************************************************************************
// Config
//****************************************************************************

template<class T>
ConfigVar_ptr<T> Config::Lookup(const std::string& name, const T& value, const std::string& description) {
	RWMutexType::WriteLock lock(GetMutex());
	auto it = GetDatas().find(name);
	if (it != GetDatas().end()) {
		auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
		if (tmp) {
			SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name = " << name << " exists ";
			return tmp;
		} else {
			SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name = " << name << " exists but type not "
				<< typeid(T).name() << " real_type = " << it->second->getTypeName()
				<< " " << it->second->toString();
			return nullptr;
		}
	}

	if (name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._0123456789") != std::string::npos) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invalid " << name;
		throw std::invalid_argument(name);
	}
	ConfigVar_ptr<T> v = std::make_shared<ConfigVar<T>>(name, value, description);
	GetDatas()[name] = v;
	return v;
}

template<class T>
ConfigVar_ptr<T> Config::Lookup(const std::string& name) {
	RWMutexType::ReadLock lock(GetMutex());
	auto it = GetDatas().find(name);
	if (it == GetDatas().end()) return nullptr;
	else return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
}


}; /* sylar */

#endif /* SYLAR_CONFIG_H */
