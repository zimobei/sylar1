#include "Log.h"
#include "Single.h"

#include <iostream>
#include <unordered_map>
#include <functional>

namespace sylar {

//****************************************************************************
// LogLevel
//****************************************************************************

extern const std::unordered_map<LogLevel, std::string> __level_string_hash_map{
	{LogLevel::DEBUG, "DEBUG"},
	{LogLevel::INFO, "INFO"},
	{LogLevel::WARN, "WARN"},
	{LogLevel::ERROR, "ERROR"},
	{LogLevel::FATAL, "FATAL"},
	{LogLevel::UNKNOW, "UNKNOW"}
};

std::string LevelToString(LogLevel level) {
	auto it = __level_string_hash_map.find(level);
	if (it == __level_string_hash_map.end()) return "UNKNOW";
	else return it->second;
}

extern const std::unordered_map<std::string, LogLevel> __string_level_hash_map{
	{"DEBUG", LogLevel::DEBUG},
	{"INFO", LogLevel::INFO},
	{"WARN", LogLevel::WARN},
	{"ERROR", LogLevel::ERROR},
	{"FATAL", LogLevel::FATAL},
	{"UNKNOW", LogLevel::UNKNOW}
};

LogLevel LevelFromString(const std::string& str) {
	auto it = __string_level_hash_map.find(str);
	if (it == __string_level_hash_map.end()) return LogLevel::UNKNOW;
	else return it->second;
}

//****************************************************************************
// LogEvent
//****************************************************************************

LogEvent::LogEvent(const std::string& log_name, LogLevel level,
				   const std::string& file_name, uint32_t line,
				   uint32_t elapse, uint32_t thread_id,
				   const std::string& thread_name,
				   uint32_t fiber_id, uint64_t time)
	:
	__log_name(log_name),
	__level(level),
	__file_name(file_name),
	__line(line),
	__elapse(elapse),
	__thread_id(thread_id),
	__thread_name(thread_name),
	__fiber_id(fiber_id),
	__time(time),
	__sstream() {}

const std::string& LogEvent::getLogName() const {
	return this->__log_name;
}

const std::string& LogEvent::getFile() const {
	return this->__file_name;
}

uint32_t LogEvent::getLine() const {
	return this->__line;
}

uint32_t LogEvent::getElapse() const {
	return this->__elapse;
}

uint32_t LogEvent::getThreadId() const {
	return this->__thread_id;
}

uint32_t LogEvent::getFiberId() const {
	return this->__fiber_id;
}

const std::string& LogEvent::getThreadName() const {
	return __thread_name;
}

uint64_t LogEvent::getTime() const {
	return this->__time;
}

LogLevel LogEvent::getLevel() const {
	return this->__level;
}

std::string LogEvent::getContext() const {
	return this->__sstream.str();
}

std::stringstream& LogEvent::getSS() {
	return this->__sstream;
}


//****************************************************************************
// FormatterItem
//****************************************************************************

class MessageFormatItem : public FormatterItem {
public:
	MessageFormatItem(const std::string& str = "") {}
	void format(std::ostream& os, LogEvent_ptr event) override {
		os << event->getContext();
	}
};

class LevelFormatItem : public FormatterItem {
public:
	LevelFormatItem(const std::string& str = "") {}
	void format(std::ostream& os, LogEvent_ptr event) override {
		os << LevelToString(event->getLevel());
	}
};

class ElapseFormatItem : public FormatterItem {
public:
	ElapseFormatItem(const std::string& str = "") {}
	void format(std::ostream& os, LogEvent_ptr event) override {
		os << event->getElapse();
	}
};

class NameFormatItem : public FormatterItem {
public:
	NameFormatItem(const std::string& str = "") {}
	void format(std::ostream& os, LogEvent_ptr event) override {
		os << event->getLogName();
	}
};

class ThreadIdFormatItem : public FormatterItem {
public:
	ThreadIdFormatItem(const std::string& str = "") {}
	void format(std::ostream& os, LogEvent_ptr event) override {
		os << event->getThreadId();
	}
};

class ThreadNameFormatItem : public FormatterItem {
public:
	ThreadNameFormatItem(const std::string& str = "") {}
	void format(std::ostream& os, LogEvent_ptr event) override {
		os << event->getThreadName();
	}
};

class FiberIdFormatItem : public FormatterItem {
public:
	FiberIdFormatItem(const std::string& str = "") {}
	void format(std::ostream& os, LogEvent_ptr event) override {
		os << event->getFiberId();
	}
};

class DateTimeFormatItem : public FormatterItem {
public:
	DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
		:m_format(format) {
		if (m_format.empty()) {
			m_format = "%Y-%m-%d %H:%M:%S";
		}
	}

	void format(std::ostream& os, LogEvent_ptr event) override {
		time_t time = event->getTime();
		char tmp[64];
		struct tm* timinfo;
		timinfo = localtime(&time);
		strftime(tmp, sizeof(tmp), m_format.c_str(), timinfo);
		os << tmp;
	}
private:
	std::string m_format;
};

class FilenameFormatItem : public FormatterItem {
public:
	FilenameFormatItem(const std::string& str = "") {}
	void format(std::ostream& os, LogEvent_ptr event) override {
		os << event->getFile();
	}
};

class LineFormatItem : public FormatterItem {
public:
	LineFormatItem(const std::string& str = "") {}
	void format(std::ostream& os, LogEvent_ptr event) override {
		os << event->getLine();
	}
};

class NewLineFormatItem : public FormatterItem {
public:
	NewLineFormatItem(const std::string& str = "") {}
	void format(std::ostream& os, LogEvent_ptr event) override {
		os << std::endl;
	}
};

class StringFormatItem : public FormatterItem {
public:
	StringFormatItem(const std::string& str)
		:m_string(str) {}
	void format(std::ostream& os, LogEvent_ptr event) override {
		os << m_string;
	}
private:
	std::string m_string;
};

class TabFormatItem : public FormatterItem {
public:
	TabFormatItem(const std::string& str = "") {}
	void format(std::ostream& os, LogEvent_ptr event) override {
		os << "\t";
	}
private:
	std::string m_string;
};

//****************************************************************************
// LogFormatter
//****************************************************************************

LogFormatter::LogFormatter(const std::string& pattern) :__pattern(pattern) {
	init();
}

extern const std::unordered_map<std::string, std::function<FormatterItem_ptr(const std::string& str)>> __format_items_hash_map {
		{ "m", [](const std::string& fmt) {return std::make_shared<MessageFormatItem>(fmt); } },
		{ "p", [](const std::string& fmt) {return std::make_shared<LevelFormatItem>(fmt); } },
		{ "r", [](const std::string& fmt) {return std::make_shared<ElapseFormatItem>(fmt); } },
		{ "c", [](const std::string& fmt) {return std::make_shared<NameFormatItem>(fmt); } },
		{ "t", [](const std::string& fmt) {return std::make_shared<ThreadIdFormatItem>(fmt); } },
		{ "n", [](const std::string& fmt) {return std::make_shared<NewLineFormatItem>(fmt); } },
		{ "d", [](const std::string& fmt) {return std::make_shared<DateTimeFormatItem>(fmt); } },
		{ "f", [](const std::string& fmt) {return std::make_shared<FilenameFormatItem>(fmt); } },
		{ "l", [](const std::string& fmt) {return std::make_shared<LineFormatItem>(fmt); } },
		{ "T", [](const std::string& fmt) {return std::make_shared<TabFormatItem>(fmt); } },
		{ "F", [](const std::string& fmt) {return std::make_shared<FiberIdFormatItem>(fmt); } },
		{ "N", [](const std::string& fmt) {return std::make_shared<ThreadNameFormatItem>(fmt); } }
};

void LogFormatter::init() {
	std::vector<std::tuple<std::string, std::string, int>> vec;
	std::string nstr; // 解析后的字符串
	// 循环解析
	for (std::size_t i = 0; i < __pattern.size(); ++i) {
		// 如果不是 % 号，则添加该字符
		if (__pattern[i] != '%') {
			nstr.push_back(__pattern[i]);
			continue;
		}

		// m_pattern[i] == % && m_pattern[i + 1] == '%'
		// 此时第二个字符被视作普通字符
		if ((i + 1) < __pattern.size() && __pattern[i + 1] == '%') {
			nstr.push_back('%');
			continue;
		}

		// m_pattern[i]是% && m_pattern[i + 1] != '%'
		std::size_t n = i + 1;		// 跳过'%',从'%'的下一个字符开始解析
		int fmt_status = 0;			// 是否解析大括号内的内容: 已经遇到'{',但是还没有遇到'}' 值为1
		std::size_t fmt_begin = 0;	// 大括号开始的位置

		std::string str;
		std::string fmt;			// 存放'{}'中间截取的字符
		while (n < __pattern.size()) {
			// __pattern[n]不是字母 && __pattern[n]不是'{' && __pattern[n]不是'}'
			if (!fmt_status && (!isalpha(__pattern[n]) && __pattern[n] != '{' && __pattern[n] != '}')) {
				str = __pattern.substr(i + 1, n - i - 1);
				break;
			}

			if (fmt_status == 0) {
				if (__pattern[n] == '{') {
					str = __pattern.substr(i + 1, n - i - 1);
					fmt_status = 1;
					fmt_begin = n;
					++n;
					continue;
				}
			} else if (fmt_status == 1) {
				if (__pattern[n] == '}') {
					fmt = __pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
					fmt_status = 0;
					++n;
					break;
				}
			}
			++n;
			if (n == __pattern.size() && str.empty()) {
				str = __pattern.substr(i + 1);
			}
		}

		if (fmt_status == 0) {
			if (!nstr.empty()) {
				vec.push_back(std::make_tuple(nstr, std::string(), 0));
				nstr.clear();
			}
			vec.push_back(std::make_tuple(str, fmt, 1));
			i = n - 1;
		} else if (fmt_status == 1) {
			vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
		}
	}

	if (!nstr.empty()) {
		vec.push_back(std::make_tuple(nstr, "", 0));
	}

	for (auto& i : vec) {
		if (std::get<2>(i) == 0) {
			__items.push_back(std::make_shared<StringFormatItem>(std::get<0>(i)));
		} else {
			auto it = __format_items_hash_map.find(std::get<0>(i));
			if (it == __format_items_hash_map.end()) {
				__items.push_back(std::make_shared<StringFormatItem>("<<error_format %" + std::get<0>(i) + ">>"));
			} else {
				__items.push_back(it->second(std::get<1>(i)));
			}
		}
	}
}

std::string LogFormatter::format(LogEvent_ptr event) {
	std::stringstream sstream;
	for (auto& item : __items) {
		item->format(sstream, event);
	}
	return sstream.str();
}

//****************************************************************************
// LogAppender
//****************************************************************************

void LogAppender::setFormatter(LogFormatter_ptr val) {
	SpinLock::Lock lock(__mutex);
	this->__formatter = val;
}

LogFormatter_ptr LogAppender::getFormatter() {
	SpinLock::Lock lock(__mutex);
	return this->__formatter;
}

void StdOutLogAppender::log(LogEvent_ptr event) {
	SpinLock::Lock lock(__mutex);
	std::cout << this->__formatter->format(event);
}

bool FileLogAppender::reopen() {
	SpinLock::Lock lock(__mutex);
	if (__file_stream) __file_stream.close();
	__file_stream.open(__file_name, std::ios::app);
	return !!__file_stream;
}

FileLogAppender::FileLogAppender(const std::string& file_name) 
	: __file_name(file_name) {
	reopen();
}

void FileLogAppender::log(LogEvent_ptr event) {
	SpinLock::Lock lock(__mutex);
	__file_stream << __formatter->format(event);
}

//****************************************************************************
// Logger
//****************************************************************************

Logger::Logger(const std::string& name) :__name(name), __level(LogLevel::DEBUG) {}

// 一个输出日志的方法(传入想要查看的最大日志级别)
void Logger::log(LogEvent_ptr event) {
	if (event->getLevel() >= __level) {
		SpinLock::Lock lock(__mutex);
		for (auto& i : __appenders) {
			i->log(event);
		}
	}
}

const std::string& Logger::getName() const {
	return this->__name;
}

LogLevel Logger::getLevel() const {
	return this->__level;
}

void Logger::setLevel(LogLevel val) {
	this->__level = val;
}

void Logger::addAppender(LogAppender_ptr appender) {
	SpinLock::Lock lock(__mutex);
	this->__appenders.push_back(appender);
}

void Logger::delAppender(LogAppender_ptr appender) {
	SpinLock::Lock lock(__mutex);
	for (auto it = __appenders.begin(); it != __appenders.end(); ++it) {
		if (*it == appender) {
			__appenders.erase(it);
			break;
		}
	}
}

//****************************************************************************
// LogEventWrap
//****************************************************************************

LogEventWrap::LogEventWrap(Logger_ptr logger, LogEvent_ptr event_ptr)
	:__logger(logger), __event_ptr(event_ptr) {
}

LogEventWrap::~LogEventWrap() {
	this->__logger->log(__event_ptr);
}

LogEvent_ptr LogEventWrap::getEvent() const {
	return this->__event_ptr;
}

std::stringstream& LogEventWrap::getSS() {
	return this->__event_ptr->getSS();
}

//****************************************************************************
// LoggerManager
//****************************************************************************

LoggerManager::LoggerManager() : __root(std::make_shared<Logger>()) {
	LogFormatter_ptr formatter = std::make_shared<LogFormatter>();
	StdOutLogAppender_ptr std_out_ptr = std::make_shared<StdOutLogAppender>();
	std_out_ptr->setFormatter(formatter);
	FileLogAppender_ptr file_out_ptr = std::make_shared<FileLogAppender>("log.txt");
	file_out_ptr->setFormatter(formatter);
	__root->addAppender(std_out_ptr);
	__root->addAppender(file_out_ptr);
}

Logger_ptr LoggerManager::getLogger(const std::string& name) {
	SpinLock::Lock lock(__mutex);
	auto it = __loggers.find(name);
	if (it == __loggers.end()) return __root;
	else return it->second;
}

Logger_ptr LoggerManager::getRoot() const {
	return __root;
}

Logger_ptr SYLAR_LOG_ROOT() {
	return Single<LoggerManager>::GetInstance()->getRoot();
}

Logger_ptr SYLAR_LOG_NAME(const std::string& name) {
	return Single<LoggerManager>::GetInstance()->getLogger(name);
}

}; /* sylar */
