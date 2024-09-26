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
	// ��������ȡkey�������Ƿ����key�������������֮ǰ�޸�Ϊ���ļ��м��ص�ֵ
		std::string key = i.first;
		if (key.empty()) continue;
		// ��keyתΪСд
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);

		// ��ѯ�Ƿ����key
		ConfigVarBase_ptr var = LookupBase(key);

		// �������key�Ŵ��ļ��м��ظ��£�������ֱ������
		if (var) {
			if (i.second.IsScalar()) {
				// ��YAML::�ڽ��ֵתΪScalar����
				// Ȼ����ַ����м��أ���ͨ��ʵ��ƫ�ػ�ʵ�������͵�ת����������__val�����и���
				var->fromString(i.second.Scalar());
			} else {
				// �������� Sequence,ƫ�ػ���fromstd::string�ж�Ӧ�Ĵ�����
				std::stringstream ss;
				ss << i.second;
				var->fromString(ss.str());
			}
		}
	}
}


}; /* sylar */