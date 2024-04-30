#pragma once

#include "noncopyable.h"

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

/*
   每个宏都会调用Logger实例对象的相应接口,设置日志级别并输出日志信息.
   其中,LOG_FATAL宏还会在输出日志信息后调用exit(-1)函数退出程序.
   并且,LOG_DEBUG在MUDEBUG编译选项下才会有输出,默认没有输出.

   对于Logger类,需要注意的是,Logger类的构造函数是私有的,即不能直接创建Logger对象,这是确保Logger类的唯一性.

 */

// LOG_INFO("%s %d",arg1,arg2...)
#define LOG_INFO(logmsgFormat, ...)                       \
	do                                                    \
	{                                                     \
		Logger &logger = Logger::instance();              \
		logger.setLogLevel(INFO);                         \
		char buf[1024] = {0};                             \
		snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
		logger.log(buf);                                  \
	} while (0)

#define LOG_ERROR(logmsgFormat, ...)                      \
	do                                                    \
	{                                                     \
		Logger &logger = Logger::instance();              \
		logger.setLogLevel(ERROR);                        \
		char buf[1024] = {0};                             \
		snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
		logger.log(buf);                                  \
	} while (0)

#define LOG_FATAL(logmsgFormat, ...)                      \
	do                                                    \
	{                                                     \
		Logger &logger = Logger::instance();              \
		logger.setLogLevel(FATAL);                        \
		char buf[1024] = {0};                             \
		snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
		logger.log(buf);                                  \
		exit(-1);                                         \
	} while (0)

#ifdef MUDEBUG
#define LOG_DEBUG(logmsgFormat, ...)                      \
	do                                                    \
	{                                                     \
		Logger &logger = Logger::instance();              \
		logger.setLogLevel(DEBUG);                        \
		char buf[1024] = {0};                             \
		snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
		logger.log(buf);                                  \
	} while (0)
#else
#define LOG_DEBUG(logmsgFormat, ...)
#endif

// 定义日志的级别
enum LogLevel
{
	INFO = 0,  // 普通信息
	ERROR, 	   // 普通错误信息
	FATAL, 	   // core信息
	DEBUG  	   // 调试信息
};

class Logger
{
public:
	///////////////////terminal///////////////////////
	// 获取日志类唯一的实例对象
	static Logger &instance();
	// 设置日志级别
	void setLogLevel(int level);
	// 写日志接口
	void log(std::string msg);
	///////////////////local///////////////////////
	// 设置日志文件名
	void setLogName(const std::string &logName);
	// 异步写本地日志
	void asyncWrite(int logLevel, const std::string& format, ...);
	// 停止thread
	void stop();

private:
	int logLevel_;
	std::string logName_;
	Logger();
	void processTask(std::string task);	// 处理字符串任务

	bool stop_;
	std::condition_variable cond_;
	std::mutex mutex_;
	std::queue<std::string> queue_;
};

