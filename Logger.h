#pragma once

#include <string>

#include "noncopyable.h"

// LOG_INFO("%s %d", arg1, arg2)
#define LOG_INFO(logmsgFormat, ...) \
    do  \
    {   \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(INFO); \
        char buf[1024] = {0};   \
        snprintf(buf,1024,logmsgFormat, ##__VA_ARGS__);\
    } while (0);

#define LOG_ERROR(logmsgFormat, ...) \
    do  \
    {   \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(ERROR); \
        char buf[1024] = {0};   \
        snprintf(buf,1024,logmsgFormat, ##__VA_ARGS__);  \
    } while (0);

#define LOG_FATAL(logmsgFormat, ...) \
    do  \
    {   \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(FATAL); \
        char buf[1024] = {0};   \
        snprintf(buf,1024,logmsgFormat, ##__VA_ARGS__);  \
    } while (0);

#ifdef MODEDEBUG
#define LOG_DEBUG(logmsgFormat, ...) \
    do  \
    {   \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(DEBUG); \
        char buf[1024] = {0};   \
        snprintf(buf,1024,logmsgFormat, ##__VA_ARGS__);  \
    } while (0);
#else
    #define LOG_DEBUG(logmsgFormat, ...)
#endif

enum LogLevel
{
    INFO,   //普通信息
    ERROR,  //错误信息
    FATAL,  //core信息
    DEBUG   //调试信息
};

class Logger: noncopyable
{
public:
    static Logger& instance();      // 获取静态唯一实例
    void setLogLevel(int level);    // 设置日志等级
    void log(std::string msg);      // 写日志
private:
    Logger() = default;
    int logLevel_;
};