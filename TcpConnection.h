#pragma once

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"
#include <memory>
#include <atomic>

class EventLoop;
class Socket;
class Channel;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop,const std::string nameArg, int sockfd,const InetAddress &localAddr, const InetAddress &peerAddr);
    ~TcpConnection();

    void send(const char *msg, int len);
    void send(const std::string &buf);
    void shutdown();

    bool connected() const{ return state_ == KConnected; }
    void connectEstablished();
    void connectDestroyed();

    void setConnectionCallbck(const ConnectionCallbck &cb) { connectionCallbck_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { WriteCompleteCallback_ = cb; }
    void setCloseCallback(const CloseCallback &cb) { CloseCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, std::size_t highWaterMark) { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

private:
    enum StateE { KDisconnected, KConnecting, KConnected, KDisconnecting };
    void setState(StateE state) { state_ = state; }
    // Channel所需要处理的事件类型
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleError();
    void handleClose();

    void sendInLoop(const char *msg, int len);
    void shutdownInLoop();

    EventLoop *loop_;   // must be subLoop
    std::string name_;
    std::atomic_int state_;
    bool reading;

    std::unique_ptr<Socket> socket_;    // 与客户端通信的connfd 
    std::unique_ptr<Channel> channel_;  // 封装fd位Channle被Poller监听

    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    // 从TcpServer传来的回调
    ConnectionCallbck connectionCallbck_;
    MessageCallback messageCallback_;
    WriteCompleteCallback WriteCompleteCallback_;
    CloseCallback CloseCallback_;    
    HighWaterMarkCallback highWaterMarkCallback_; // 设置发送方的水位线，保证发送和接收的稳定
    std::size_t highWaterMark_;

    Buffer inputBuffer_;    // 接收缓冲区
    Buffer outputBuffer_;   // 发送缓冲区
};