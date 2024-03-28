#include "EventLoop.h"
#include "Logger.h"
#include "Channel.h"
#include "Poller.h"

#include <sys/eventfd.h>
#include <sys/fcntl.h>
#include <errno.h>

__thread EventLoop *t_loopInThisThread = nullptr;   // 每个线程独享一个loop
const int kPollTimeMs = 10000;  // 默认IO服用接口超时时间

// eventfd() 从内核态通知用户态
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_FATAL("eventfd create error: %d \n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop() 
    :looping_(false), quit_(false),callingPendingFunctor_(false),threadId_(CurrentThread::tid())
    ,wakeupFd_(createEventfd()),wakeupChannel_(new Channel(this,wakeupFd_))
    ,poller_(Poller::newDefaultPoller(this))
{
    LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_);
    if(t_loopInThisThread) {
        LOG_FATAL("Another EventLoop %p exists in this thread %d \n", this, threadId_);   
    }else{
        t_loopInThisThread = this;
    }
    // 用来等待唤醒的操作
    wakeupChannel_->setReadEventCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();

}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    ::close(wakeupFd_);
    wakeupChannel_->remove();
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    LOG_INFO("EventLoop %p start looping. \n", this);
    looping_ = true;
    quit_ = false;
    // 循环poll获取活跃的channel并处理event
    while (!quit_) {
        Timestamp timestamp = poller_->poll(kPollTimeMs,activeChannels_);
        for(Channel* channel: *activeChannels_) {
            channel->handleEvent(timestamp);
        }
        doPendingFunctor();
    }
    looping_ = false;
    LOG_INFO("EventLoop %p stop looping. \n", this);
}

void EventLoop::quit()
{
    quit_ = false;
    if (!isInLoopThread())  // 如果不是在loop自己线程中quit,则唤醒loop线程
        wakeup();
}

void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread()) {
        cb();
    }else{
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }
    // callingPendingFunctor_ 若是正在处理,那么loop()处理后又会堵塞在poll(),故需要向wakeupFd写数据再次唤醒
    if(!isInLoopThread() || callingPendingFunctor_) {   
        wakeup();
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    int n = write(wakeupFd_, &one, sizeof one);
    if(n != sizeof one) {
        LOG_ERROR("func : %s => writes %d bytes instead of 8", __FUNCTION__, n);
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    int n = read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_FATAL("func : %s => reads %d bytes instead of 8", __FUNCTION__, n);
    }
}

void EventLoop::doPendingFunctor()
{
    callingPendingFunctor_ = true;
    std::vector<Functor> functors;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);    // 直接交换数据,防止pendingFunctors无法持续接受数据
    }

    for(auto &functor: functors) {
        functor();
    }
    callingPendingFunctor_ = false;
}
