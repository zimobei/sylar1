#ifndef SYLAR_TEST_YAML_H
#define SYLAR_TEST_YAML_H

#include <string>
#include <fstream>
#include <iostream>

#include <yaml-cpp/yaml.h>

using std::string;
using std::ofstream;
using std::cout;
using std::endl;

namespace Test
{

// 读取文件的路径
const string FileName = R"(test\test_yaml.yaml)";

// yaml-cpp 读文件测试
void test_yaml_read() {
	cout << "--------- test yaml read -------------" << endl;

	YAML::Node node = YAML::LoadFile(FileName);
	cout << node["name"].as<string>() << endl;
	cout << node["sex"].as<string>() << endl;
	cout << node["age"].as<int>() << endl;
	cout << node["system"]["port"].as<string>() << endl;
	cout << node["system"]["value"].as<string>() << endl;
	for (auto it = node["system"]["vecs"].begin(); it != node["system"]["vecs"].end(); ++it) {
		cout << *it << "  ";
	}
	cout << endl;
}

// yaml-cpp 写文件测试
void test_yaml_write() {
	cout << "--------- test yaml write -------------" << endl;

	// 创建一个包含数字、字符串和数组数据的YAML节点
	YAML::Node data;
	data["name"] = "John Doe";
	data["sex"] = "man";
	data["age"] = 30;
	data["system"]["port"] = 88;
	data["system"]["value"] = 100;

	// 创建一个包含数组的YAML节点
	YAML::Node hobbies;
	hobbies.push_back(1);
	hobbies.push_back(2);
	hobbies.push_back(3);
	data["system"]["vecs"] = hobbies;

	 // 将数据写入文件
	ofstream file(FileName);
	file << data;
	file.close();
}

void test_yaml() {
	cout << "--------- test yaml -------------" << endl;
	test_yaml_write();
	test_yaml_read();
	cout << endl << endl;
	cout << "--------- test over -------------" << endl;
	cout << endl;
}

}; /* Test */

#endif /* SYLAR_TEST_YAML_H */
