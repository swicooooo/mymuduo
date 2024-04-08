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

/// @brief 
class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop,const std::string nameArg, int sockfd,const InetAddress &localAddr, const InetAddress &peerAddr);
    ~TcpConnection();

    void send(const std::string &buf);
    void shutdown();

    void connectEstablished();
    void connectDestroyed();

    void setConnectionCallbck(const ConnectionCallbck &cb) { connectionCallbck_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
    void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, std::size_t highWaterMark) { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

    bool connected() const{ return state_ == KConnected; }
    EventLoop* getLoop() const{ return loop_; }
    std::string name() const{ return name_; }
    const InetAddress peerAddress() const { return peerAddr_; }
    const InetAddress localAddress() const { return localAddr_; }
private:
    enum StateE { KDisconnected, KConnecting, KConnected, KDisconnecting };
    void setState(StateE state) { state_ = state; }
    // TcpConnection为Channel设置所需要处理的事件类型的实际回调
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
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;    
    HighWaterMarkCallback highWaterMarkCallback_; // 设置发送方的水位线，保证发送和接收的稳定
    std::size_t highWaterMark_;

    Buffer inputBuffer_;    // 接收缓冲区
    Buffer outputBuffer_;   // 发送缓冲区
};