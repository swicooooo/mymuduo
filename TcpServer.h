#pragma once

#include "Callbacks.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"

#include <atomic>
#include <memory>
#include <unordered_map>

class TcpServer
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
    enum Option {
        KNoReusePort,
        KReusePort
    };
    TcpServer(EventLoop *loop, InetAddress &listenAddr, const std::string &nameArg,Option option = KNoReusePort);
    ~TcpServer();

    void start();   // 开启服务监听
    void setThreadNums(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }

    void setConnectionCallbck(const ConnectionCallbck &cb) { connectionCallbck_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { WriteCompleteCallback_ = cb; }

private:
    void newConnection(int sockfd, const InetAddress &listenAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    EventLoop *loop_;   //baseLoop,用户提供
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;    // 用于处理新连接
    std::shared_ptr<EventLoopThreadPool> threadPool_;   // 用于处理后续数据交互
    
    // 用户行为触发相应回调函数
    ConnectionCallbck connectionCallbck_;
    MessageCallback messageCallback_;
    WriteCompleteCallback WriteCompleteCallback_;

    ThreadInitCallback threadInitCallback_; // loop初始时的回调
    std::atomic_int started_;   // 确保只启动一次
    int nextConnId;
    ConnectionMap connectionMap_;   // 保存所有连接
};