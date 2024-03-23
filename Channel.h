#pragma once

#include "noncopyable.h"
#include "TimeStamp.h"

#include <functional>
#include <memory>

class EventLoop;

/**
 * like -> Demultiplex
 * Channel 封装了sockfd和其相关event，如EPOLLIN、EPOLLOUT，以及绑定了poller需要返回的具体事件
*/
class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using KReadEventCallback = std::function<void(TimeStamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    // 处理fd得到poller通知的事件
    void handleEvent(TimeStamp receiveTime);
    
    // 设置回调函数对象
    void setKReadEventCallback(KReadEventCallback cb);
    void setKWriteEventCallback(EventCallback cb);
    void setCloseEventCallback(EventCallback cb);
    void setErrorEventCallback(EventCallback cb);
    
    // 防止channel被手动romove后，还在执行回调函数
    void tie(const std::shared_ptr<void>);

    // 设置fd相关的事件状态
    void enableReading() { event_ |= KReadEvent; update(); }
    void disableReading() { event_ &= ~KReadEvent; update(); }
    void enableWriting() { event_ |= KWriteEvent; update(); }
    void disableWriting() { event_ &= ~KWriteEvent; update(); }
    void disableAll() { event_ = KNoneEvent; update(); }

    // 返回当前fd事件状态
    bool isKNoneEvent()const { return KNoneEvent==event_; }
    bool isReading()const { return KReadEvent & event_; }
    bool isWriting()const { return KWriteEvent & event_; }

    // 外界访问私有变量
    int fd() const;
    int event() const;
    void set_revent(int rvt);
    int index()const;
    void set_index(int idx);
    EventLoop* ownerLoop()const;
    void remove();
private:
    void update();
    void handleEventWithGuard(TimeStamp receiveTime);

    static const int KNoneEvent;
    static const int KReadEvent;
    static const int KWriteEvent;

    EventLoop *loop_;
    const int fd_;
    int event_;     // fd相关事件
    int revent_;    // poller返回的具体发生的事件
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;

    KReadEventCallback KReadEventCallback_;
    EventCallback KWriteEventCallback_;
    EventCallback closeEventCallback_;
    EventCallback errorEventCallback_;
};