#pragma once

#include <atomic>
#include <functional>
#include <vector>
#include <memory>
#include <mutex>

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

class Channel;
class Poller;

/// @brief 
///                     mainReactor
///     subReactor1     subReactor2     subReactor3
/// 这里Reactor是通过eventfd直接通信,or可以添加一个线程安全队列隔绝
/// 执行EventLoop里面投入的回调函数
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();

    void loop();    // 开启事件循环
    void quit();    // 退出循环

    // 当mainLoop调用subLoop的方法时和subLoop调用另一个subLoop方法时使用
    void runInLoop(Functor cb);     // 在当前loop中执行cb
    void queueInLoop(Functor cb);   // 将cb放入队列，唤醒loop所在线程并执行cb
    void wakeup();  // 向wakeup随便写一个数据来唤醒loop所在线程

    Timestamp pollReturnTime() const{ return pollReturnTime_; }
    // 判断loop是否在自己的线程中，用于区分mainLoop和subLoop，
    // 当mainLoop监听到新连接时，会轮询选择一个subLoop唤醒执行newConnection回调
    bool isInLoopThread() const{ return threadId_ == CurrentThread::tid(); } 

    // 执行poller中的Channel操作
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);
private:
    void handleRead();          // 执行wakeup
    void doPendingFunctor();    // 执行回调

    using ChannelList = std::vector<Channel*>;
    std::atomic_bool looping_;  // 标识是否循环
    std::atomic_bool quit_;     // 标识是否退出

    const pid_t threadId_;      // 当前线程id

    Timestamp pollReturnTime_;  // poller返回发生事件时的时间戳
    std::unique_ptr<Poller> poller_;    // 管理的poller,监听clientfd和wakeupfd

    int wakeupFd_;  // 当mainReactor建立连接后，轮询选择后，通过该成员唤醒对应的subReactor
    std::unique_ptr<Channel> wakeupChannel_;    // Reactor用来通知subReator的Channel
    ChannelList *activeChannels_;   // 活跃的Channel

    std::atomic_bool callingPendingFunctor_;    // 标识是否还有未处理的cb
    std::vector<Functor> pendingFunctors_;      // 存储loop未处理的cb集
    std::mutex mutex_;  // 保证cb集的线程安全
};