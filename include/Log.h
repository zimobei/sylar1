//*****************************************************************************
//
//
//   此头文件实现日志系统功能
//  
//
//*****************************************************************************

#ifndef SYLAR_LOG_H
#define SYLAR_LOG_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <fstream>

#include "Util.h"
#include "Mutex.h"

namespace sylar{

//****************************************************************************
// 前置声明
//****************************************************************************

class LogEvent;
using LogEvent_ptr = typename std::shared_ptr<LogEvent>;

class LogFormatter;
using LogFormatter_ptr = typename std::shared_ptr<LogFormatter>;

class FormatterItem;
using FormatterItem_ptr = typename std::shared_ptr<FormatterItem>;

class LogAppender;
using LogAppender_ptr = typename std::shared_ptr<LogAppender>;

class StdOutLogAppender;
using StdOutLogAppender_ptr = typename std::shared_ptr<StdOutLogAppender>;

class FileLogAppender;
using FileLogAppender_ptr = typename std::shared_ptr<FileLogAppender>;

class Logger;
using Logger_ptr = typename std::shared_ptr<Logger>;

class LogEventWrap;

class LoggerManager;

//****************************************************************************
// 日志级别
//****************************************************************************

enum LogLevel {
	UNKNOW = 0,		// 未知级别日志
	DEBUG = 1,		// 调试级别日志
	INFO = 2,		// 普通级别日志
	WARN = 3,		// 警告级别日志
	ERROR = 4,		// 错误级别日志
	FATAL = 5		// 灾难级别日志
};

/*!
 * @brief 将日志级别转成文本输出
 */
std::string LevelToString(LogLevel level);

/*!
 * @brief 将文本转换成日志级别
 */
LogLevel LevelFromString(const std::string& str);


//****************************************************************************
// 日志信息
//****************************************************************************

class LogEvent {
private: // 成员变量
	//日志器名称
	std::string __log_name;
	//日志级别
	LogLevel __level;
	//文件名
	std::string __file_name;
	//行号
	uint32_t __line;
	//程序启动开始到现在的毫秒数
	uint32_t __elapse;
	//线程id
	uint32_t __thread_id;
	//协程id
	uint32_t __fiber_id;
	//时间戳
	uint64_t __time;
	//线程名
	std::string __thread_name;
	//字符流
	std::stringstream __sstream;
public: // 构造函数
	LogEvent(const std::string& log_name, LogLevel level,
			 const std::string& file_name, uint32_t line,
			 uint32_t elapse, uint32_t thread_id,
			 const std::string& thread_name,
			 uint32_t fiber_id, uint64_t time);
public: // 接口
	/*!
	 * @brief 返回日志器名称
	 */
	const std::string& getLogName() const;

	/*!
	 * @brief 返回文件名
	 */
	const std::string& getFile() const;

	/*!
	 * @brief 返回行号
	 */
	uint32_t getLine() const;

	/*!
	 * @brief 返回程序启动开始到现在的毫秒数
	 */
	uint32_t getElapse() const;

	/*!
	 * @brief 返回线程id
	 */
	uint32_t getThreadId() const;

	/*!
	 * @brief 返回协程id
	 */
	uint32_t getFiberId() const;

	/*!
	 * @brief 返回线程名称
	 */
	const std::string& getThreadName() const;

	/*!
	 * @brief 返回时间戳
	 */
	uint64_t getTime() const;

	/*!
	 * @brief 返回日志级别
	 */
	LogLevel getLevel() const;

	/*!
	 * @brief 返回字符流中内容
	 */
	std::string getContext() const;

	/*!
	 * @brief 返回字符流
	 */
	std::stringstream& getSS();

};

//****************************************************************************
// 日志格式化
//****************************************************************************

static const std::string __dafault_formatter = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n";

// 抽象接口
class FormatterItem {
public:
	virtual ~FormatterItem() {}
	virtual void format(std::ostream& os, LogEvent_ptr event) = 0;
};

class LogFormatter {
private: // 成员变量
	std::string __pattern;
	std::vector<FormatterItem_ptr> __items;
private:
	void init();
public: // 构造函数
	LogFormatter(const std::string& pattern = __dafault_formatter);
public:
	std::string format(LogEvent_ptr event);
};
 
//****************************************************************************
// 日志输出
//****************************************************************************

class LogAppender {
protected:
	LogFormatter_ptr __formatter;
	SpinLock __mutex;
public:
	virtual ~LogAppender() {}
	virtual void log(LogEvent_ptr event) = 0;
	void setFormatter(LogFormatter_ptr val);
	LogFormatter_ptr getFormatter();
};

class StdOutLogAppender : public LogAppender {
public:
	void log(LogEvent_ptr event) override;
};

class FileLogAppender : public LogAppender {
private:
	// 文件路径
	std::string __file_name;
	// 文件流
	std::ofstream __file_stream;
public:
	FileLogAppender(const std::string& filename);
	void log(LogEvent_ptr event) override;
	/*!
	 * @brief 重新打开日志文件 
	 */
	bool reopen();
};

//****************************************************************************
// 日志器
//****************************************************************************

class Logger {
private:
	std::string __name;
	LogLevel __level; // 本日志器能够输出的最大日志级别
	SpinLock __mutex;
	std::list<LogAppender_ptr> __appenders;
public:
	Logger(const std::string& name = "root");

	// 一个输出日志的方法(传入想要查看的最大日志级别)
	void log(LogEvent_ptr event);

	const std::string& getName() const;
	LogLevel getLevel() const;
	void setLevel(LogLevel val);

	void addAppender(LogAppender_ptr appender);
	void delAppender(LogAppender_ptr appender);
};

//****************************************************************************
// 管理类 及其相关工具宏
//****************************************************************************

class LoggerManager {
private:
	std::map<std::string, Logger_ptr> __loggers;
	Logger_ptr __root;
	SpinLock __mutex;
public:
	LoggerManager();
	Logger_ptr getLogger(const std::string& name);
	Logger_ptr getRoot() const;
};

Logger_ptr SYLAR_LOG_ROOT();

Logger_ptr SYLAR_LOG_NAME(const std::string& name);

//****************************************************************************
// RAII 管理 LogEvent 的输出
//****************************************************************************

class LogEventWrap {
private:
	Logger_ptr __logger;
	LogEvent_ptr __event_ptr;
public:
	LogEventWrap(Logger_ptr logger, LogEvent_ptr e);
	~LogEventWrap();

	LogEvent_ptr getEvent() const;
	std::stringstream& getSS();
};

//****************************************************************************
// 使用流式方式将日志级别level的日志写入到logger
//****************************************************************************

#define SYLAR_LOG_LEVEL(logger, level) \
	if (logger->getLevel() <= level) \
 		LogEventWrap(logger, LogEvent_ptr(new LogEvent( \
				logger->getName(), level, __FILE__, __LINE__, 0, \
                GetThreadId(), "log_thread", 1, time(0)))) \
      	.getSS() \
  		

#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, LogLevel::DEBUG)

#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, LogLevel::INFO)

#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, LogLevel::WARN)

#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, LogLevel::ERROR)

#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, LogLevel::FATAL)

}; /* sylar */

#endif /* SYLAR_LOG_H */
