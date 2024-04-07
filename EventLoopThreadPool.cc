#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg)
    :baseLoop_(baseLoop), name_(nameArg), started_(false),numThreads_(0), next_(0)
    {}

void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    started_ = true;

    // 多loop时, 创建多个EventLoopThread
    for (size_t i = 0; i < numThreads_; i++) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s %ld", name_.c_str(), i);
        EventLoopThread *t = new EventLoopThread(cb,buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));    // 不能使用make_unique和t 会导致赋值构造
        loops_.push_back(t->startLoop());
    }
    
    // 只有一个baseLoop_时
    if(numThreads_ == 0 && cb) {
        cb(baseLoop_);
    }
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    EventLoop *loop = baseLoop_;
    if(!loops_.empty()) {
        loop = loops_[next_++];
        if(next_ >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}
