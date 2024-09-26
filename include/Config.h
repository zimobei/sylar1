//*****************************************************************************
//
//
//   ��ͷ�ļ�ʵ������ϵͳ����
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
// ǰ������
//****************************************************************************

class ConfigVarBase;
using ConfigVarBase_ptr = std::shared_ptr<ConfigVarBase>;

template<class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
class ConfigVar;

template<class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
using ConfigVar_ptr = std::shared_ptr<ConfigVar<T, FromStr, ToStr>>;

using ConfigVarMap = std::map<std::string, ConfigVarBase_ptr>;

//****************************************************************************
// ������
//****************************************************************************

/*!
 * @brief ���ñ����Ļ���
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
	 * @brief ���������Ϣ
	 */
	virtual std::string toString() = 0;

	/*!
	 * @brief �������ļ�����Ϣת��Ϊ ConfigVarBase ������Ϣ
	 */
	virtual bool fromString(const std::string& val) = 0;

	/*!
	 * @brief �������ò���ֵ����������
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
	 * @brief ������ֵת����YAML std::string
	 * @exception ��ת��ʧ���׳��쳣 
	 */
	std::string toString() override;

	/*!
	 * @brief ��YAML std::string ת�ɲ�����ֵ
	 * @exception ��ת��ʧ���׳��쳣
	 */
	bool fromString(const std::string& val) override;

	/*!
	 * @brief ���� ConfigVar<T> �� ���� T �� name
	 */
	std::string getTypeName() const override;

	/*!
	 * @brief ����ֵ��ʱ�򣬼���ֵ�Ƿ�������������仯������Ӧ�Ĳ���
	 */
	void setValue(const T& val);

	/*!
	 * @brief ����ֵ
	 */
	const T& getValue();

	/*!
	 * @brief ���Ӽ���
	 */
	uint64_t addListener(on_change_cb cb);

	/*!
	 * @brief ɾ������
	 */
	void delListener(uint64_t key);

	/*!
	 * @brief ��ü�����
	 */
	on_change_cb getListener(uint64_t key);

	/*!
	 * @brief ��ռ�����
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
// ����Ϊģ�����Լ�ģ�庯����ʵ��
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
		// �Ƚ��������Ҫ���Զ�����������
		if (val == __val) return;
		for (auto& i : __cbs) {
			// ����ִ�лص������������ڹ۲���ģʽ
			i.second(__val, val);
		}
	}
	RWMutexType::WriteLock lock(__mutex);
	// ��ֵ
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
