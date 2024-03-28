#pragma once

#include <thread>
#include <memory> 
#include <atomic>
#include <string>
#include <functional>

class Thread
{
public:
    using ThreadFunc = std::function<void()>;
    Thread(ThreadFunc func, const std::string &name = std::string());
    ~Thread();

    void start();   // 一个Thread开启一个新线程,并存储详细信息
    void join();    

    bool started() const{ return started_; }
    pid_t tid() const{ return tid_; }
    const std::string& name() const{ return name_; } 
    const int numCreated() const{ return numCreated_; }
private:
    void setDefaultName();
    
    bool started_;
    bool joined_;
    pid_t tid_;
    std::shared_ptr<std::thread> thread_;   // 用智能指针包裹thread,控制启动时间
    ThreadFunc func_;
    std::string name_;
    static std::atomic_int32_t numCreated_;
};