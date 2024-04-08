#include "EventLoopThread.h"
#include "Thread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &func, const std::string &name)
    :exiting_(false), loop_(nullptr),
    thread_(std::bind(&EventLoopThread::ThreadFunc, this), name),threadInitCallback_(func){}

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
    thread_.start(); // 执行thread绑定的回调EventLoopThread::ThreadFunc
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop == nullptr) { // 等待EventLoop创建成功
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::ThreadFunc()
{
    EventLoop loop;
    if(threadInitCallback_) {
        threadInitCallback_(&loop);
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