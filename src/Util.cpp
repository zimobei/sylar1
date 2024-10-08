#include "Util.h"
#include "Log.h"
#include "Fiber.h"
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <sstream>
#include <signal.h>
#include <stdarg.h>


namespace sylar{

pid_t GetThreadId() {
    return syscall(SYS_gettid);
}

uint32_t GetFiberId() {
    return Fiber::GetFiberId();
}

std::string demangle(const char* str) {
    std::string rt;
    rt.resize(256);
    if (sscanf(str, "%*[^(]%*[^_]%255[^)+]", &rt[0]) == 1) {
        std::size_t size = 0;
        int status = 0;
        char* v = abi::__cxa_demangle(&rt[0], nullptr, &size, &status);
        if (v) {
            std::string result(v);
            free(v);
            return result;
        }
    }
    
    if (sscanf(str, "%255s", &rt[0]) == 1) return rt;

    return str;
}

std::vector<std::string> Backtrace(std::size_t size, std::size_t skip) {
    std::vector<std::string> result;
    void** array = (void**)malloc(size * sizeof(void*));
    std::size_t s = ::backtrace(array, size);
    char** strings = backtrace_symbols(array, s);
    if (strings != nullptr) {
        for (std::size_t i = skip; i < s; ++i) {
            result.emplace_back(demangle(strings[i]));
        }
    }
    else {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "backtrace_symbols error";
    }
    free(strings);
    free(array);
    return result;
}

void Backtrace(std::vector<std::string>& bt, int size, int skip) {
    for (const std::string& str : Backtrace(size, skip)) {
        bt.emplace_back(str);
    }
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
    std::vector<std::string> strs = Backtrace(size, skip);
    std::stringstream ss;
    for (std::size_t i = 0; i < strs.size(); ++i) {
        ss << prefix << strs[i].c_str() << std::endl;
    }
    return ss.str();
}

uint64_t GetCurrentMS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}

uint64_t GetCurrentUS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
}

std::string Time2Str(time_t ts, const std::string& format) {
    struct tm tm;
    localtime_r(&ts, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), format.c_str(), &tm);
    return buf;
}

time_t Str2Time(const char* str, const char* format) {
    struct tm t;
    memset(&t, 0, sizeof(t));
    if (!strptime(str, format, &t)) {
        return 0;
    }
    return mktime(&t);
}

//****************************************************************************
// FSUtil
//****************************************************************************

void FSUtil::ListAllFile(std::vector<std::string>& files,
                         const std::string& path,
                         const std::string& subfix) {
    if (access(path.c_str(), 0) != 0) {
        return;
    }
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        return;
    }
    struct dirent* dp = nullptr;
    while ((dp = readdir(dir)) != nullptr) {
        if (dp->d_type == DT_DIR) {
            if (!strcmp(dp->d_name, ".")
                || !strcmp(dp->d_name, "..")) {
                continue;
            }
            ListAllFile(files, path + "/" + dp->d_name, subfix);
        }
        else if (dp->d_type == DT_REG) {
            std::string filename(dp->d_name);
            if (subfix.empty()) {
                files.push_back(path + "/" + filename);
            }
            else {
                if (filename.size() < subfix.size()) {
                    continue;
                }
                if (filename.substr(filename.length() - subfix.size()) == subfix) {
                    files.push_back(path + "/" + filename);
                }
            }
        }
    }
    closedir(dir);
}

static int __lstat(const char* file, struct stat* st = nullptr) {
    struct stat lst;
    int ret = lstat(file, &lst);
    if (st) {
        *st = lst;
    }
    return ret;
}

static int __mkdir(const char* dirname) {
    if (access(dirname, F_OK) == 0) {
        return 0;
    }
    return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

bool FSUtil::Mkdir(const std::string& dirname) {
    if (__lstat(dirname.c_str()) == 0) {
        return true;
    }
    char* path = strdup(dirname.c_str());
    char* ptr = strchr(path + 1, '/');
    do {
        for (; ptr; *ptr = '/', ptr = strchr(ptr + 1, '/')) {
            *ptr = '\0';
            if (__mkdir(path) != 0) {
                break;
            }
        }
        if (ptr != nullptr) {
            break;
        }
        else if (__mkdir(path) != 0) {
            break;
        }
        free(path);
        return true;
    } while (0);
    free(path);
    return false;
}

bool FSUtil::IsRunningPidfile(const std::string& pidfile) {
    if (__lstat(pidfile.c_str()) != 0) {
        return false;
    }
    std::ifstream ifs(pidfile);
    std::string line;
    if (!ifs || !std::getline(ifs, line)) {
        return false;
    }
    if (line.empty()) {
        return false;
    }
    pid_t pid = atoi(line.c_str());
    if (pid <= 1) {
        return false;
    }
    if (kill(pid, 0) != 0) {
        return false;
    }
    return true;
}

bool FSUtil::Rm(const std::string& path) {
    struct stat st;
    if (lstat(path.c_str(), &st)) {
        return true;
    }
    if (!(st.st_mode & S_IFDIR)) {
        return Unlink(path);
    }

    DIR* dir = opendir(path.c_str());
    if (!dir) {
        return false;
    }

    bool ret = true;
    struct dirent* dp = nullptr;
    while ((dp = readdir(dir))) {
        if (!strcmp(dp->d_name, ".")
            || !strcmp(dp->d_name, "..")) {
            continue;
        }
        std::string dirname = path + "/" + dp->d_name;
        ret = Rm(dirname);
    }
    closedir(dir);
    if (::rmdir(path.c_str())) {
        ret = false;
    }
    return ret;
}

bool FSUtil::Mv(const std::string& from, const std::string& to) {
    if (!Rm(to)) {
        return false;
    }
    return rename(from.c_str(), to.c_str()) == 0;
}

bool FSUtil::Realpath(const std::string& path, std::string& rpath) {
    if (__lstat(path.c_str())) {
        return false;
    }
    char* ptr = ::realpath(path.c_str(), nullptr);
    if (nullptr == ptr) {
        return false;
    }
    std::string(ptr).swap(rpath);
    free(ptr);
    return true;
}

bool FSUtil::Symlink(const std::string& from, const std::string& to) {
    if (!Rm(to)) {
        return false;
    }
    return ::symlink(from.c_str(), to.c_str()) == 0;
}

bool FSUtil::Unlink(const std::string& filename, bool exist) {
    if (!exist && __lstat(filename.c_str())) {
        return true;
    }
    return ::unlink(filename.c_str()) == 0;
}

std::string FSUtil::Dirname(const std::string& filename) {
    if (filename.empty()) {
        return ".";
    }
    auto pos = filename.rfind('/');
    if (pos == 0) {
        return "/";
    }
    else if (pos == std::string::npos) {
        return ".";
    }
    else {
        return filename.substr(0, pos);
    }
}

std::string FSUtil::Basename(const std::string& filename) {
    if (filename.empty()) {
        return filename;
    }
    auto pos = filename.rfind('/');
    if (pos == std::string::npos) {
        return filename;
    }
    else {
        return filename.substr(pos + 1);
    }
}

bool FSUtil::OpenForRead(std::ifstream& ifs, const std::string& filename,
                         std::ios_base::openmode mode) {
    ifs.open(filename.c_str(), mode);
    return ifs.is_open();
}

bool FSUtil::OpenForWrite(std::ofstream& ofs, const std::string& filename,
                          std::ios_base::openmode mode) {
    ofs.open(filename.c_str(), mode);
    if (!ofs.is_open()) {
        std::string dir = Dirname(filename);
        Mkdir(dir);
        ofs.open(filename.c_str(), mode);
    }
    return ofs.is_open();
}

////****************************************************************************
//// TypeUtil
////****************************************************************************
//
//int8_t TypeUtil::ToChar(const std::string& str) {
//    if (str.empty()) {
//        return 0;
//    }
//    return *str.begin();
//}
//
//int64_t TypeUtil::Atoi(const std::string& str) {
//    if (str.empty()) {
//        return 0;
//    }
//    return strtoull(str.c_str(), nullptr, 10);
//}
//
//double TypeUtil::Atof(const std::string& str) {
//    if (str.empty()) {
//        return 0;
//    }
//    return atof(str.c_str());
//}
//
//int8_t TypeUtil::ToChar(const char* str) {
//    if (str == nullptr) {
//        return 0;
//    }
//    return str[0];
//}
//
//int64_t TypeUtil::Atoi(const char* str) {
//    if (str == nullptr) {
//        return 0;
//    }
//    return strtoull(str, nullptr, 10);
//}
//
//double TypeUtil::Atof(const char* str) {
//    if (str == nullptr) {
//        return 0;
//    }
//    return atof(str);
//}

//****************************************************************************
// StringUtil
//****************************************************************************

std::string StringUtil::Format(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    auto v = Formatv(fmt, ap);
    va_end(ap);
    return v;
}

std::string StringUtil::Formatv(const char* fmt, va_list ap) {
    char* buf = nullptr;
    auto len = vasprintf(&buf, fmt, ap);
    if (len == -1) {
        return "";
    }
    std::string ret(buf, len);
    free(buf);
    return ret;
}

static const char uri_chars[256] = {
    /* 0 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 0, 0, 0, 1, 0, 0,
    /* 64 */
    0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,
    /* 128 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    /* 192 */
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
};

static const char xdigit_chars[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
    0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

#define CHAR_IS_UNRESERVED(c)           \
    (uri_chars[(unsigned char)(c)])

//-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~
std::string StringUtil::UrlEncode(const std::string& str, bool space_as_plus) {
    static const char* hexdigits = "0123456789ABCDEF";
    std::string* ss = nullptr;
    const char* end = str.c_str() + str.length();
    for (const char* c = str.c_str(); c < end; ++c) {
        if (!CHAR_IS_UNRESERVED(*c)) {
            if (!ss) {
                ss = new std::string;
                ss->reserve(str.size() * 1.2);
                ss->append(str.c_str(), c - str.c_str());
            }
            if (*c == ' ' && space_as_plus) {
                ss->append(1, '+');
            }
            else {
                ss->append(1, '%');
                ss->append(1, hexdigits[(uint8_t)*c >> 4]);
                ss->append(1, hexdigits[*c & 0xf]);
            }
        }
        else if (ss) {
            ss->append(1, *c);
        }
    }
    if (!ss) {
        return str;
    }
    else {
        std::string rt = *ss;
        delete ss;
        return rt;
    }
}

std::string StringUtil::UrlDecode(const std::string& str, bool space_as_plus) {
    std::string* ss = nullptr;
    const char* end = str.c_str() + str.length();
    for (const char* c = str.c_str(); c < end; ++c) {
        if (*c == '+' && space_as_plus) {
            if (!ss) {
                ss = new std::string;
                ss->append(str.c_str(), c - str.c_str());
            }
            ss->append(1, ' ');
        }
        else if (*c == '%' && (c + 2) < end
                 && isxdigit(*(c + 1)) && isxdigit(*(c + 2))) {
            if (!ss) {
                ss = new std::string;
                ss->append(str.c_str(), c - str.c_str());
            }
            ss->append(1, (char)(xdigit_chars[(int)*(c + 1)] << 4 | xdigit_chars[(int)*(c + 2)]));
            c += 2;
        }
        else if (ss) {
            ss->append(1, *c);
        }
    }
    if (!ss) {
        return str;
    }
    else {
        std::string rt = *ss;
        delete ss;
        return rt;
    }
}

std::string StringUtil::Trim(const std::string& str, const std::string& delimit) {
    auto begin = str.find_first_not_of(delimit);
    if (begin == std::string::npos) {
        return "";
    }
    auto end = str.find_last_not_of(delimit);
    return str.substr(begin, end - begin + 1);
}

std::string StringUtil::TrimLeft(const std::string& str, const std::string& delimit) {
    auto begin = str.find_first_not_of(delimit);
    if (begin == std::string::npos) {
        return "";
    }
    return str.substr(begin);
}

std::string StringUtil::TrimRight(const std::string& str, const std::string& delimit) {
    auto end = str.find_last_not_of(delimit);
    if (end == std::string::npos) {
        return "";
    }
    return str.substr(0, end);
}

std::string StringUtil::WStringToString(const std::wstring& ws) {
    std::string str_locale = setlocale(LC_ALL, "");
    const wchar_t* wch_src = ws.c_str();
    size_t n_dest_size = wcstombs(NULL, wch_src, 0) + 1;
    char* ch_dest = new char[n_dest_size];
    memset(ch_dest, 0, n_dest_size);
    wcstombs(ch_dest, wch_src, n_dest_size);
    std::string str_result = ch_dest;
    delete[]ch_dest;
    setlocale(LC_ALL, str_locale.c_str());
    return str_result;
}

std::wstring StringUtil::StringToWString(const std::string& s) {
    std::string str_locale = setlocale(LC_ALL, "");
    const char* chSrc = s.c_str();
    size_t n_dest_size = mbstowcs(NULL, chSrc, 0) + 1;
    wchar_t* wch_dest = new wchar_t[n_dest_size];
    wmemset(wch_dest, 0, n_dest_size);
    mbstowcs(wch_dest, chSrc, n_dest_size);
    std::wstring wstr_result = wch_dest;
    delete[]wch_dest;
    setlocale(LC_ALL, str_locale.c_str());
    return wstr_result;
}

////****************************************************************************
//// JsonUtil
////****************************************************************************
//
//bool YamlToJson(const YAML::Node& ynode, Json::Value& jnode) {
//    try {
//        if (ynode.IsScalar()) {
//            Json::Value v(ynode.Scalar());
//            jnode.swapPayload(v);
//            return true;
//        }
//        if (ynode.IsSequence()) {
//            for (size_t i = 0; i < ynode.size(); ++i) {
//                Json::Value v;
//                if (YamlToJson(ynode[i], v)) {
//                    jnode.append(v);
//                }
//                else {
//                    return false;
//                }
//            }
//        }
//        else if (ynode.IsMap()) {
//            for (auto it = ynode.begin();
//                 it != ynode.end(); ++it) {
//                Json::Value v;
//                if (YamlToJson(it->second, v)) {
//                    jnode[it->first.Scalar()] = v;
//                }
//                else {
//                    return false;
//                }
//            }
//        }
//    }
//    catch (...) {
//        return false;
//    }
//    return true;
//}
//
//bool JsonToYaml(const Json::Value& jnode, YAML::Node& ynode) {
//    try {
//        if (jnode.isArray()) {
//            for (int i = 0; i < (int)jnode.size(); ++i) {
//                YAML::Node n;
//                if (JsonToYaml(jnode[i], n)) {
//                    ynode.push_back(n);
//                }
//                else {
//                    return false;
//                }
//            }
//        }
//        else if (jnode.isObject()) {
//            for (auto it = jnode.begin();
//                 it != jnode.end();
//                 ++it) {
//                YAML::Node n;
//                if (JsonToYaml(*it, n)) {
//                    ynode[it.name()] = n;
//                }
//                else {
//                    return false;
//                }
//            }
//        }
//        else {
//            ynode = jnode.asString();
//        }
//    }
//    catch (...) {
//        return false;
//    }
//    return true;
//}
//
//bool JsonUtil::NeedEscape(const std::string& v) {
//    for (auto& c : v) {
//        switch (c) {
//            case '\f':
//            case '\t':
//            case '\r':
//            case '\n':
//            case '\b':
//            case '"':
//            case '\\':
//                return true;
//            default:
//                break;
//        }
//    }
//    return false;
//}
//
//std::string JsonUtil::Escape(const std::string& v) {
//    size_t size = 0;
//    for (auto& c : v) {
//        switch (c) {
//            case '\f':
//            case '\t':
//            case '\r':
//            case '\n':
//            case '\b':
//            case '"':
//            case '\\':
//                size += 2;
//                break;
//            default:
//                size += 1;
//                break;
//        }
//    }
//    if (size == v.size()) {
//        return v;
//    }
//
//    std::string rt;
//    rt.resize(size);
//    for (auto& c : v) {
//        switch (c) {
//            case '\f':
//                rt.append("\\f");
//                break;
//            case '\t':
//                rt.append("\\t");
//                break;
//            case '\r':
//                rt.append("\\r");
//                break;
//            case '\n':
//                rt.append("\\n");
//                break;
//            case '\b':
//                rt.append("\\b");
//                break;
//            case '"':
//                rt.append("\\\"");
//                break;
//            case '\\':
//                rt.append("\\\\");
//                break;
//            default:
//                rt.append(1, c);
//                break;
//
//        }
//    }
//    return rt;
//}
//
//std::string JsonUtil::GetString(const Json::Value& json, 
//                                const std::string& name, 
//                                const std::string& default_value) {
//    if (!json.isMember(name)) {
//        return default_value;
//    }
//    auto& v = json[name];
//    if (v.isString()) {
//        return v.asString();
//    }
//    return default_value;
//}
//
//double JsonUtil::GetDouble(const Json::Value& json,
//                           const std::string& name,
//                           double default_value) {
//    if (!json.isMember(name)) {
//        return default_value;
//    }
//    auto& v = json[name];
//    if (v.isDouble()) {
//        return v.asDouble();
//    }
//    else if (v.isString()) {
//        return TypeUtil::Atof(v.asString());
//    }
//    return default_value;
//}
//
//int32_t JsonUtil::GetInt32(const Json::Value& json,
//                           const std::string& name,
//                           int32_t default_value) {
//    if (!json.isMember(name)) {
//        return default_value;
//    }
//    auto& v = json[name];
//    if (v.isInt()) {
//        return v.asInt();
//    }
//    else if (v.isString()) {
//        return TypeUtil::Atoi(v.asString());
//    }
//    return default_value;
//}
//
//uint32_t JsonUtil::GetUint32(const Json::Value& json,
//                             const std::string& name,
//                             uint32_t default_value) {
//    if (!json.isMember(name)) {
//        return default_value;
//    }
//    auto& v = json[name];
//    if (v.isUInt()) {
//        return v.asUInt();
//    }
//    else if (v.isString()) {
//        return TypeUtil::Atoi(v.asString());
//    }
//    return default_value;
//}
//
//int64_t JsonUtil::GetInt64(const Json::Value& json,
//                           const std::string& name,
//                           int64_t default_value) {
//    if (!json.isMember(name)) {
//        return default_value;
//    }
//    auto& v = json[name];
//    if (v.isInt64()) {
//        return v.asInt64();
//    }
//    else if (v.isString()) {
//        return TypeUtil::Atoi(v.asString());
//    }
//    return default_value;
//}
//
//uint64_t JsonUtil::GetUint64(const Json::Value& json,
//                             const std::string& name,
//                             uint64_t default_value) {
//    if (!json.isMember(name)) {
//        return default_value;
//    }
//    auto& v = json[name];
//    if (v.isUInt64()) {
//        return v.asUInt64();
//    }
//    else if (v.isString()) {
//        return TypeUtil::Atoi(v.asString());
//    }
//    return default_value;
//}
//
//bool JsonUtil::FromString(Json::Value& json, const std::string& v) {
//    Json::Reader reader;
//    return reader.parse(v, json);
//}
//
//std::string JsonUtil::ToString(const Json::Value& json) {
//    Json::FastWriter w;
//    return w.write(json);
//}

}; /* sylar */