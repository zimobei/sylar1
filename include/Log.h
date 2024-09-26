//*****************************************************************************
//
//
//   ��ͷ�ļ�ʵ����־ϵͳ����
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
// ǰ������
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
// ��־����
//****************************************************************************

enum LogLevel {
	UNKNOW = 0,		// δ֪������־
	DEBUG = 1,		// ���Լ�����־
	INFO = 2,		// ��ͨ������־
	WARN = 3,		// ���漶����־
	ERROR = 4,		// ���󼶱���־
	FATAL = 5		// ���Ѽ�����־
};

/*!
 * @brief ����־����ת���ı����
 */
std::string LevelToString(LogLevel level);

/*!
 * @brief ���ı�ת������־����
 */
LogLevel LevelFromString(const std::string& str);


//****************************************************************************
// ��־��Ϣ
//****************************************************************************

class LogEvent {
private: // ��Ա����
	//��־������
	std::string __log_name;
	//��־����
	LogLevel __level;
	//�ļ���
	std::string __file_name;
	//�к�
	uint32_t __line;
	//����������ʼ�����ڵĺ�����
	uint32_t __elapse;
	//�߳�id
	uint32_t __thread_id;
	//Э��id
	uint32_t __fiber_id;
	//ʱ���
	uint64_t __time;
	//�߳���
	std::string __thread_name;
	//�ַ���
	std::stringstream __sstream;
public: // ���캯��
	LogEvent(const std::string& log_name, LogLevel level,
			 const std::string& file_name, uint32_t line,
			 uint32_t elapse, uint32_t thread_id,
			 const std::string& thread_name,
			 uint32_t fiber_id, uint64_t time);
public: // �ӿ�
	/*!
	 * @brief ������־������
	 */
	const std::string& getLogName() const;

	/*!
	 * @brief �����ļ���
	 */
	const std::string& getFile() const;

	/*!
	 * @brief �����к�
	 */
	uint32_t getLine() const;

	/*!
	 * @brief ���س���������ʼ�����ڵĺ�����
	 */
	uint32_t getElapse() const;

	/*!
	 * @brief �����߳�id
	 */
	uint32_t getThreadId() const;

	/*!
	 * @brief ����Э��id
	 */
	uint32_t getFiberId() const;

	/*!
	 * @brief �����߳�����
	 */
	const std::string& getThreadName() const;

	/*!
	 * @brief ����ʱ���
	 */
	uint64_t getTime() const;

	/*!
	 * @brief ������־����
	 */
	LogLevel getLevel() const;

	/*!
	 * @brief �����ַ���������
	 */
	std::string getContext() const;

	/*!
	 * @brief �����ַ���
	 */
	std::stringstream& getSS();

};

//****************************************************************************
// ��־��ʽ��
//****************************************************************************

static const std::string __dafault_formatter = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n";

// ����ӿ�
class FormatterItem {
public:
	virtual ~FormatterItem() {}
	virtual void format(std::ostream& os, LogEvent_ptr event) = 0;
};

class LogFormatter {
private: // ��Ա����
	std::string __pattern;
	std::vector<FormatterItem_ptr> __items;
private:
	void init();
public: // ���캯��
	LogFormatter(const std::string& pattern = __dafault_formatter);
public:
	std::string format(LogEvent_ptr event);
};
 
//****************************************************************************
// ��־���
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
	// �ļ�·��
	std::string __file_name;
	// �ļ���
	std::ofstream __file_stream;
public:
	FileLogAppender(const std::string& filename);
	void log(LogEvent_ptr event) override;
	/*!
	 * @brief ���´���־�ļ� 
	 */
	bool reopen();
};

//****************************************************************************
// ��־��
//****************************************************************************

class Logger {
private:
	std::string __name;
	LogLevel __level; // ����־���ܹ�����������־����
	SpinLock __mutex;
	std::list<LogAppender_ptr> __appenders;
public:
	Logger(const std::string& name = "root");

	// һ�������־�ķ���(������Ҫ�鿴�������־����)
	void log(LogEvent_ptr event);

	const std::string& getName() const;
	LogLevel getLevel() const;
	void setLevel(LogLevel val);

	void addAppender(LogAppender_ptr appender);
	void delAppender(LogAppender_ptr appender);
};

//****************************************************************************
// ������ ������ع��ߺ�
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
// RAII ���� LogEvent �����
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
// ʹ����ʽ��ʽ����־����level����־д�뵽logger
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
