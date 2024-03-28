#include "EventLoopThread.h"
#include "Thread.h"
#include "EventLoop.h"
#include <mutex>

EventLoopThread::EventLoopThread(ThreadInitCallback func, const std::string &name)
    :exiting_(false), loop_(nullptr),
    thread_(std::bind(&EventLoopThread::ThreadFunc, this), name),callback_(func){}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    thread_.start();
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop == nullptr) {
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::ThreadFunc()
{
    EventLoop loop;
    if(callback_) {
        callback_(&loop);
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();
    std::unique_lock<std::mutex> lock(mutex_); // 保护资源
    loop_ = nullptr;
}