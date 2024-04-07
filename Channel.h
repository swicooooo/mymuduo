#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>

class EventLoop;

/// @brief Channel用于封装sockfd、设置sockfd感兴趣事件和处理事件回调(TcpConnection时设置)
/// 执行具体的回调，sock相关的api（listen\accept）由Socket类完成
class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel() = default;

    // 处理fd得到poller通知的事件
    void handleEvent(Timestamp receiveTime);
    
    // 防止channel执行事件回调时TcpConnection被析构
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
    int events() const   { return events_; }
    void setRevent(int rvt) { revents_ = rvt; }
    int index() const   { return index_; }
    void setIndex(int idx)  { index_ = idx; }
    EventLoop *ownerLoop() const { return loop_; }
    void remove();
private:
    void update();  // 通过poller更新fd的事件状态
    void handleEventWithGuard(Timestamp receiveTime);   //根据poller返回的事件，执行对应的回调函数

    // sockfd相关事件
    static const int KNoneEvent;
    static const int KReadEvent;
    static const int KWriteEvent;

    EventLoop *loop_;//指针只占四字节(32), 只需要前置声明,不需要包含头文件
    const int fd_;
    int events_;     // fd相关事件
    int revents_;    // poller返回的具体发生的事件
    int index_;      // 对应Channel的状态

    std::weak_ptr<void> tie_;   // 持有TcpConnection引用
    bool tied_;

    ReadEventCallback readEventCallback_;
    EventCallback writeEventCallback_;
    EventCallback closeEventCallback_;
    EventCallback errorEventCallback_;
};