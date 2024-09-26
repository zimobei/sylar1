//*****************************************************************************
//
//
//   此头文件封装常用工具函数
//  
//
//*****************************************************************************

#ifndef SYLAR_UTIL_H
#define SYLAR_UTIL_H

#include <pthread.h>
#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <yaml-cpp/yaml.h>
#include <jsoncpp/json/json.h>

namespace sylar{

/*!
 * @brief 返回当前线程 ID
 */
pid_t GetThreadId();

/*!
 * @brief 获取当前协程 ID
 */
uint32_t GetFiberId();

/*!
 * @brief 获取当前的调用栈
 * @param bt 保存调用栈信息
 * @param size 最多返回层数
 * @param skip 跳过栈顶的层数
 * @result 保存调用栈信息
 */
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
std::vector<std::string> Backtrace(std::size_t size = 64, std::size_t skip = 1);

/*!
 * @brief 获取当前栈信息的字符串
 * @param size 栈的最大层数
 * @param skip 跳过栈顶的层数
 * @param prefix 每条栈信息前输出的内容
 */
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

/*!
 * @brief 获取当前时间（毫秒）
 */
uint64_t GetCurrentMS();

/*!
 * @brief 获取当前时间（微妙）
 */
uint64_t GetCurrentUS();

std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");
time_t Str2Time(const char* str, const char* format = "%Y-%m-%d %H:%M:%S");

class FSUtil {
public:
    static void ListAllFile(std::vector<std::string>& files, 
                            const std::string& path, 
                            const std::string& subfix);
    static bool Mkdir(const std::string& dirname);
    static bool IsRunningPidfile(const std::string& pidfile);
    static bool Rm(const std::string& path);
    static bool Mv(const std::string& from, const std::string& to);
    static bool Realpath(const std::string& path, std::string& rpath);
    static bool Symlink(const std::string& from, const std::string& to);
    static bool Unlink(const std::string& filename, bool exist = false);
    static std::string Dirname(const std::string& filename);
    static std::string Basename(const std::string& filename);
    static bool OpenForRead(std::ifstream& ifs, const std::string& filename, 
                            std::ios_base::openmode mode);
    static bool OpenForWrite(std::ofstream& ofs, const std::string& filename, 
                             std::ios_base::openmode mode);
};

//class TypeUtil {
//public:
//    static int8_t ToChar(const std::string& str);
//    static int64_t Atoi(const std::string& str);
//    static double Atof(const std::string& str);
//    static int8_t ToChar(const char* str);
//    static int64_t Atoi(const char* str);
//    static double Atof(const char* str);
//};

class StringUtil {
public:
    static std::string Format(const char* fmt, ...);
    static std::string Formatv(const char* fmt, va_list ap);

    static std::string UrlEncode(const std::string& str, bool space_as_plus = true);
    static std::string UrlDecode(const std::string& str, bool space_as_plus = true);

    static std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimLeft(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimRight(const std::string& str, const std::string& delimit = " \t\r\n");


    static std::string WStringToString(const std::wstring& ws);
    static std::wstring StringToWString(const std::string& s);

};

//bool YamlToJson(const YAML::Node& ynode, Json::Value& jnode);
//bool JsonToYaml(const Json::Value& jnode, YAML::Node& ynode);
//
//class JsonUtil {
//public:
//    static bool NeedEscape(const std::string& v);
//    static std::string Escape(const std::string& v);
//    static std::string GetString(const Json::Value& json, const std::string& name, const std::string& default_value = "");
//    static double GetDouble(const Json::Value& json, const std::string& name, double default_value = 0);
//    static int32_t GetInt32(const Json::Value& json, const std::string& name, int32_t default_value = 0);
//    static uint32_t GetUint32(const Json::Value& json, const std::string& name, uint32_t default_value = 0);
//    static int64_t GetInt64(const Json::Value& json, const std::string& name, int64_t default_value = 0);
//    static uint64_t GetUint64(const Json::Value& json, const std::string& name, uint64_t default_value = 0);
//    static bool FromString(Json::Value& json, const std::string& v);
//    static std::string ToString(const Json::Value& json);
//};

}; /* sylar */

#endif /* SYLAR_UTIL_H */
