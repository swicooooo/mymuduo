#pragma once

#include <atomic>
#include <functional>
#include <sched.h>
#include <vector>
#include <memory>
#include <mutex>

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

class Channel;
class Poller;

/**
 * like -> Reactor (与Demultiplex交互)
*/
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();

    void loop();    // 开启事件循环
    void quit();    // 退出循环

    Timestamp pollReturnTime() const{ return pollReturnTime_; }
    bool isInLoopThread() const{ return threadId_ == CurrentThread::cacheTid(); }   // 判断loop是否在自己的线程中

    void runInLoop(Functor cb);     // 在当前loop中执行cb
    void queueInLoop(Functor cb);   // 将cb放入队列，唤醒loop所在线程并执行cb
    void wakeup();  // 唤醒loop所在线程

    // 执行poller中的Channel操作
    void updateChannel();
    void removeChannel();
    bool hasChannel();
private:
    void handleRead();          // 执行wakeup
    void doPendingFunctor();    // 执行回调

    using ChannelList = std::vector<Channel*>;
    std::atomic_bool looping_;  // 标识是否循环
    std::atomic_bool quit_;     // 标识是否退出

    const pid_t threadId_;      // 当前线程id

    Timestamp pollReturnTime_;  // poller返回发生事件时的时间戳
    std::unique_ptr<Poller> poller_;    // 管理的poller

    int wakeupFd_;  // 当mainReactor建立连接后，轮询选择后，通过该成员唤醒对应的subReactor
    std::unique_ptr<Channel> wakeupChannel_;    // 
    ChannelList *activeChannels_;   // 活跃的Channel

    std::atomic_bool callingPendingFunctor_;    // 标识是否还有未处理的cb
    std::vector<Functor> pendingFunctors_;      // 存储loop未处理的cb集
    std::mutex mutex_;  // 保证cb集的线程安全
};