#ifndef SYLAR_TEST_CONFIG_H
#define SYLAR_TEST_CONFIG_H

#include "Log.h"
#include "LexicalCast.h"
#include "Config.h"

#include <iostream>
#include <string>
#include <fstream>
#include <yaml-cpp/yaml.h>
namespace sylar{

class Person {
public:
    std::string __name;
    int __age;
    bool __sex;

    Person() : __name("abc"), __age(1), __sex(false){}

    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name = " << __name
            << " age = " << __age
            << " sex = " << __sex << "]";
        return ss.str();
    }

    bool operator==(const Person& other) const {
        return __name == other.__name && __age == other.__age && __sex == other.__sex;
    }
};

/**
  *数据类型转换-person偏特化版本
  * string -> person
  */
template <>
class LexicalCast<std::string, Person> {
public:
    Person operator()(const std::string& v) {
        // loads the input string as a single YAML document
        YAML::Node node = YAML::Load(v);
        Person p;
        p.__name = node["name"].as<std::string>();
        p.__age = node["age"].as<int>();
        p.__sex = node["sex"].as<bool>();
        return p;
    }
};

/**
 * 数据类型转换-Person偏特化版本
 * Person -> string
 */
template <>
class LexicalCast<Person, std::string> {
public:
    std::string operator()(const Person& v) {
        YAML::Node node;
        node["name"] = v.__name;
        node["age"] = v.__age;
        node["sex"] = v.__sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

}; /* sylar */

using namespace sylar;

namespace Test
{

void test_config() {
    cout << "----------------------- test config ---------------------" << endl;

    ConfigVar_ptr<Person> person_ptr = Config::Lookup("my_class_person", Person(), "class person");

    // 使用lambda表达式定义回调函数
    auto func = [](const Person& old_value, const Person& new_value) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
            << endl
            << "listener function : " << endl
            << "old value = " << old_value.toString() << endl
            << "new value = " << new_value.toString() << endl;
    };

    person_ptr->addListener(func);

    const string file_name = R"(test\test_config.yaml)";
    // 写数据
    YAML::Node person_data;
    person_data["my_class_person"]["name"] = "moper";
    person_data["my_class_person"]["age"] = 10;
    person_data["my_class_person"]["sex"] = false;
    std::ofstream file(file_name);
    file << person_data;
    file.close();
    // 读数据
    YAML::Node root = YAML::LoadFile(file_name);
    // 更新数据
    Config::LoadFromYaml(root);

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT())
        << endl
        << person_ptr->getValue().toString()
        << endl
        << person_ptr->toString();

    cout << "----------------------- test over ---------------------" << endl;
    cout << endl;
}

}; /* Test */

#endif /* SYLAR_TEST_CONFIG_H */
