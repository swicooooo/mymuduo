#pragma once

#include "noncopyable.h"
#include "TimeStamp.h"

#include <muduo/net/Channel.h>

#include <functional>
#include <memory>

class EventLoop;

/**
 * Channel 封装了sockfd和其相关event，如EPOLLIN、EPOLLOUT，以及绑定了poller需要返回的具体事件
*/
class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(TimeStamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

private:
    static const int NonEvent;
    static const int ReadEvent;
    static const int WriteEvent;

    EventLoop *loop_;
    const int fd_;
    int event_;     // fd相关事件
    int revent_;    // poller返回的具体发生的事件
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;

    ReadEventCallback readEventCallback_;
    EventCallback writeEventCallback_;
    EventCallback closeEventCallback_;
    EventCallback errorEventCallback_;
};