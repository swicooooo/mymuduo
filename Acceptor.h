#pragma once

#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

#include <functional>

class EventLoop;
class InetAddress;

// Acceptor内置fd和channel， 用于baseLoop处理新连接
class Acceptor : noncopyable
{
public:
    using NewConnCallback = std::function<void(int, const InetAddress& peerAddr)>;
    Acceptor(EventLoop *loop, InetAddress &listenAddr, bool reusePort);
    ~Acceptor();

    void listen();
    bool listenning();
    void setNewConnCallback(NewConnCallback cb) { newConnCallback_ = std::move(cb); }
private:
    void handleRead();
    bool listenning_;
    EventLoop *loop_;   // baseLoop;
    Socket acceptSock_;
    Channel acceptChannel_;
    NewConnCallback newConnCallback_;
};