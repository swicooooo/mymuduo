#pragma once

#include <iostream>

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