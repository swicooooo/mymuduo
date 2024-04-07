#pragma once

#include <iostream>

/// @brief 格式化的字符串时间戳 "%F %T" yyyy-MM-dd hh:mm:ss
class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t microsecondsSinceEpoch);
    static Timestamp now(); // 获取当前时间类对象
    std::string toString(); // 将时间戳转换为字符串
private:
    int64_t microsecondsSinceEpoch_;
};