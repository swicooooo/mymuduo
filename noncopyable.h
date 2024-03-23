#pragma once

/**
 * @brief 被继承后，派生类无法进行拷贝构造和赋值操作
 * 
 */
class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};