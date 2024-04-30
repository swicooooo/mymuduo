#include "Logger.h"
#include "Timestamp.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdarg>

// 获取日志类唯一的实例对象
Logger &Logger::instance()
{
	static Logger logger;
	return logger;
}

// 设置日志级别
void Logger::setLogLevel(int level)
{
	logLevel_ = level;
}

void Logger::setLogName(const std::string &logName)
{
	logName_ = logName;
}

// 写日志接口
// 格式:[级别] time : msg
void Logger::log(std::string msg)
{
	switch (logLevel_)
	{
	case INFO:
		std::cout << "[INFO]";
		break;
	case ERROR:
		std::cout << "[ERROR]";
		break;
	case FATAL:
		std::cout << "[FATAL]";
		break;
	case DEBUG:
		std::cout << "[DEBUG]";
		break;
	default:
		break;
	}
	// 打印时间和msg -> 时间直接用muduo的timestamp
	std::cout << Timestamp::now().toString() << " : " << msg << std::endl;
}

void Logger::asyncWrite(int logLevel, const std::string &format, ...)
{
	std::stringstream ss;
	{
		va_list args;
		va_start(args, format);
		char buffer[128];
		vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
		ss << buffer;
		va_end(args);
	}
	std::lock_guard<std::mutex> lock(mutex_);
	queue_.push(ss.str());
	cond_.notify_one();
}
void Logger::stop()
{
	stop_ = true;
	cond_.notify_all();
}

Logger::Logger():stop_(false), logName_("log.txt")
{
	std::thread([this]() {
		while(true)
		{
			// 如果队列不为空，需要判断是否停止
			// 如果队列为空，但是如果停止了也要跳出wait
			std::unique_lock<std::mutex> lock(mutex_);
			cond_.wait(lock,[&](){ return !queue_.empty() || stop_; });
			if(stop_)
			{
				return;
			}
			std::string task = queue_.front();
			queue_.pop();
			lock.unlock();
			processTask(task);
		} })
		.detach();
}

void Logger::processTask(std::string task)
{
	// 做本地化存储
	task.insert(0, Timestamp::now().toString() + " ");
	std::ofstream logFile(logName_, std::ios::app);
	logFile << task << std::endl;
	logFile.flush();
	logFile.close();
}