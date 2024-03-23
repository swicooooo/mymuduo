#pragma once

#include <iostream>

class TimeStamp
{
public:
    TimeStamp();
    explicit TimeStamp(int64_t microsecondsSinceEpoch);
    static TimeStamp now(); // 获取当前时间类对象
    std::string toString(); // 将时间戳转换为字符串
private:
    int64_t microsecondsSinceEpoch_;
};