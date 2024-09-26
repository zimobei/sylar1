//*****************************************************************************
//
//
//   此头文件实现 YAML 类型与 std 类型转换功能
//  
//
//*****************************************************************************

#ifndef SYKAR_LEXICALCAST_H
#define SYKAR_LEXICALCAST_H

#include <string>
#include <vector>
#include <set>
#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>

namespace sylar {

/*!
 * @brief 将 F 类型转换为 T 类型
 */
template<class F, class T>
class LexicalCast {
public:
	T operator()(const F& v) {
		return boost::lexical_cast<T>(v);
	}
};

template<class T>
class LexicalCast<std::string, std::vector<T> > {
public:
	std::vector<T> operator()(const std::string& v) {
		YAML::Node node = YAML::Load(v);
		typename std::vector<T> vec;
		std::stringstream ss;
		for (size_t i = 0; i < node.size(); ++i) {
			ss.str("");
			ss << node[i];
			vec.push_back(LexicalCast<std::string, T>()(ss.str()));
		}
		return vec;
	}
};

template<class T>
class LexicalCast<std::vector<T>, std::string> {
public:
	std::string operator()(const std::vector<T>& v) {
		YAML::Node node;
		for (auto& i : v) {
			node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
		}
		std::stringstream ss;
		ss << node;
		return ss.str();
	}
};

template<class T>
class LexicalCast<std::string, std::list<T> > {
public:
	std::list<T> operator()(const std::string& v) {
		YAML::Node node = YAML::Load(v);
		typename std::list<T> vec;
		std::stringstream ss;
		for (size_t i = 0; i < node.size(); ++i) {
			ss.str("");
			ss << node[i];
			vec.push_back(LexicalCast<std::string, T>()(ss.str()));
		}
		return vec;
	}
};

template<class T>
class LexicalCast<std::list<T>, std::string> {
public:
	std::string operator()(const std::list<T>& v) {
		YAML::Node node;
		for (auto& i : v) {
			node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
		}
		std::stringstream ss;
		ss << node;
		return ss.str();
	}
};

template<class T>
class LexicalCast<std::string, std::set<T> > {
public:
	std::set<T> operator()(const std::string& v) {
		YAML::Node node = YAML::Load(v);
		typename std::set<T> vec;
		std::stringstream ss;
		for (size_t i = 0; i < node.size(); ++i) {
			ss.str("");
			ss << node[i];
			vec.insert(LexicalCast<std::string, T>()(ss.str()));
		}
		return vec;
	}
};

template<class T>
class LexicalCast<std::set<T>, std::string> {
public:
	std::string operator()(const std::set<T>& v) {
		YAML::Node node;
		for (auto& i : v) {
			node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
		}
		std::stringstream ss;
		ss << node;
		return ss.str();
	}
};

template<class T>
class LexicalCast<std::string, std::unordered_set<T> > {
public:
	std::unordered_set<T> operator()(const std::string& v) {
		YAML::Node node = YAML::Load(v);
		typename std::unordered_set<T> vec;
		std::stringstream ss;
		for (size_t i = 0; i < node.size(); ++i) {
			ss.str("");
			ss << node[i];
			vec.insert(LexicalCast<std::string, T>()(ss.str()));
		}
		return vec;
	}
};

template<class T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
	std::string operator()(const std::unordered_set<T>& v) {
		YAML::Node node;
		for (auto& i : v) {
			node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
		}
		std::stringstream ss;
		ss << node;
		return ss.str();
	}
};

template<class T>
class LexicalCast<std::string, std::map<std::string, T> > {
public:
	std::map<std::string, T> operator()(const std::string& v) {
		YAML::Node node = YAML::Load(v);
		typename std::map<std::string, T> vec;
		std::stringstream ss;
		for (auto it = node.begin(); it != node.end(); ++it) {
			ss.str("");
			ss << it->second;
			vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
		}
		return vec;
	}
};

template<class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
	std::string operator()(const std::map<std::string, T>& v) {
		YAML::Node node;
		for (auto& i : v) {
			node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
		}
		std::stringstream ss;
		ss << node;
		return ss.str();
	}
};

template<class T>
class LexicalCast<std::string, std::unordered_map<std::string, T> > {
public:
	std::unordered_map<std::string, T> operator()(const std::string& v) {
		YAML::Node node = YAML::Load(v);
		typename std::unordered_map<std::string, T> vec;
		std::stringstream ss;
		for (auto it = node.begin(); it != node.end(); ++it) {
			ss.str("");
			ss << it->second;
			vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
		}
		return vec;
	}
};

template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
	std::string operator()(const std::unordered_map<std::string, T>& v) {
		YAML::Node node;
		for (auto& i : v) {
			node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
		}
		std::stringstream ss;
		ss << node;
		return ss.str();
	}
};


}; /* sylar */

#endif /* SYKAR_LEXICALCAST_H */
