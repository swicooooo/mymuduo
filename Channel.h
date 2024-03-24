#pragma once

#include "noncopyable.h"
#include "TimeStamp.h"

#include <functional>
#include <memory>
#include <muduo/net/Channel.h>
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
    ~Channel() = default;

    // 处理fd得到poller通知的事件
    void handleEvent(TimeStamp receiveTime);
    
    // 防止channel被手动romove后，还在执行回调函数
    void tie(const std::shared_ptr<void>& obj);

    // 设置回调函数对象
    void setReadEventCallback(ReadEventCallback cb){ readEventCallback_ = std::move(cb); }
    void setWriteEventCallback(EventCallback cb){ writeEventCallback_ = std::move(cb); }
    void setCloseEventCallback(EventCallback cb){ errorEventCallback_ = std::move(cb); }
    void setErrorEventCallback(EventCallback cb){ closeEventCallback_ = std::move(cb); }
    
    // 设置fd相关的事件状态
    void enableReading() { events_ |= KReadEvent; update(); }
    void disableReading() { events_ &= ~KReadEvent; update(); }
    void enableWriting() { events_ |= KWriteEvent; update(); }
    void disableWriting() { events_ &= ~KWriteEvent; update(); }
    void disableAll() { events_ = KNoneEvent; update(); }

    // 返回当前fd事件状态
    bool isKNoneEvent()const { return KNoneEvent==events_; }
    bool isReading()const { return KReadEvent & events_; }
    bool isWriting()const { return KWriteEvent & events_; }

    // 外界访问私有变量
    int fd() const      { return fd_; }
    int event() const   { return events_; }
    void setRevent(int rvt) { revents_ = rvt; }
    int index() const   { return index_; }
    void setIndex(int idx)  { index_ = idx; }
    EventLoop *ownerLoop() const { return loop_; }
    void remove();
private:
    void update();  // 通过poller更新fd的事件状态
    void handleEventWithGuard(TimeStamp receiveTime);   //根据poller返回的事件，执行对应的回调函数

    static const int KNoneEvent;
    static const int KReadEvent;
    static const int KWriteEvent;

    EventLoop *loop_;//指针只占四字节(32), 只需要前置声明,不需要包含头文件
    const int fd_;
    int events_;     // fd相关事件
    int revents_;    // poller返回的具体发生的事件
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;

    ReadEventCallback readEventCallback_;
    EventCallback writeEventCallback_;
    EventCallback closeEventCallback_;
    EventCallback errorEventCallback_;
};