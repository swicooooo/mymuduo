#pragma once

#include "noncopyable.h"

#include <vector>
#include <memory>
#include <string>
#include <functional>

class EventLoop;
class EventLoopThread;

/// @brief 设置ThreadNum后开启多线程，否则只用传递进来的mainLoop
class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg);
    ~EventLoopThreadPool() = default;

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback &cb = ThreadInitCallback());
    EventLoop* getNextLoop();   // 轮询算法选择ioLoop

    bool started() const{ return started_; }
    const std::string name() const{ return name_; }
private:
    EventLoop *baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_; 
    std::vector<EventLoop*> loops_;
};