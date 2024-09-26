#include "Config.h"

namespace sylar {

//****************************************************************************
// ConfigVarBase
//****************************************************************************

ConfigVarBase::ConfigVarBase(const std::string& name, const std::string& description)
	:__name(name), __description(description) {
	std::transform(__name.begin(), __name.end(), __name.begin(), ::tolower);
}

ConfigVarBase::~ConfigVarBase() {}

const std::string& ConfigVarBase::getName() const {
	return __name;
}

const std::string& ConfigVarBase::getDescription() const {
	return __description;
}

//****************************************************************************
// Config
//****************************************************************************

static void
ListAllMember(const std::string& prefix, const YAML::Node& node,
			  std::list<std::pair<std::string, const YAML::Node>>& output) {
	if (prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._0123456789") != std::string::npos) {
		SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
		return;
	}
	output.push_back(std::make_pair(prefix, node));
	if (node.IsMap()) {
		for (auto it = node.begin(); it != node.end(); ++it) {
			std::string str = prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar();
			ListAllMember(str, it->second, output);
		}
	}
}

ConfigVarMap& Config::GetDatas() {
	static ConfigVarMap __datas;
	return __datas;
}

Config::RWMutexType& Config::GetMutex() {
	static RWMutexType __mutex;
	return __mutex;
}

ConfigVarBase_ptr Config::LookupBase(const std::string& name) {
	RWMutexType::ReadLock lock(GetMutex());
	auto it = GetDatas().find(name);
	if (it == GetDatas().end()) return nullptr;
	else return it->second;
}

void Config::LoadFromYaml(const YAML::Node& root) {
	std::list<std::pair<std::string, const YAML::Node>> all_nodes;
	ListAllMember("", root, all_nodes);

	for (auto& i : all_nodes) {
	// 遍历，获取key，查找是否包含key，如果包含，将之前修改为从文件中加载的值
		std::string key = i.first;
		if (key.empty()) continue;
		// 将key转为小写
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);

		// 查询是否包含key
		ConfigVarBase_ptr var = LookupBase(key);

		// 如果存在key才从文件中加载更新，不存在直接跳过
		if (var) {
			if (i.second.IsScalar()) {
				// 将YAML::内结点值转为Scalar类型
				// 然后从字符串中加载（已通过实现偏特化实现了类型的转换），设置__val，进行更新
				var->fromString(i.second.Scalar());
			} else {
				// 其他类型 Sequence,偏特化中fromstd::string有对应的处理方法
				std::stringstream ss;
				ss << i.second;
				var->fromString(ss.str());
			}
		}
	}
}


}; /* sylar */