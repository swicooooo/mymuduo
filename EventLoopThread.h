#pragma once

#include "noncopyable.h"
#include "Thread.h"

#include <functional>
#include <mutex>
#include <condition_variable>
#include <string>

class EventLoop;

// one loop per thread
class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback &func = ThreadInitCallback(), const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();
private:
    void ThreadFunc();
    bool exiting_;
    EventLoop *loop_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};