#include "TcpServer.h"
#include "Logger.h"

#include <string.h>

static EventLoop* checkNoNull(EventLoop *loop)
{
    if(loop == nullptr) {
        LOG_FATAL("%s:%s:%d TcpServer Loop is nil",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, InetAddress &listenAddr, const std::string &nameArg,Option option)
    : loop_(checkNoNull(loop)), 
        ipPort_(listenAddr.toIpPort()), name_(nameArg), 
        acceptor_(new Acceptor(loop,listenAddr,option==KReusePort)),
        threadPool_(new EventLoopThreadPool(loop,name_)),
        started_(0),nextConnId(1),
        connectionCallbck_(),messageCallback_()
{
    acceptor_->setNewConnCallback(std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2));
}
TcpServer::~TcpServer()
{
    for(auto &item: connections_) {
        TcpConnectionPtr conn(item.second); // 释放map中的TcpConnection强指针
        item.second.reset();
        conn->getLoop()->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::start()
{
    if(started_++ == 0) {
        threadPool_->start(threadInitCallback_);    // 启动底层的loop线程池
        loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_.get())); // 启动mainLoop开始监听新连接
    }

}
void TcpServer::setThreadNums(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    // 轮询算法，选择一个subLoop来管理channel
    EventLoop *ioLoop = threadPool_->getNextLoop();

    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(),nextConnId++);
    std::string connName = name_ + buf;
    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s \n",name_.c_str(),connName.c_str(),peerAddr.toIpPort().c_str());

    sockaddr_in local;
    ::bzero(&local, sizeof local);
    socklen_t addrLen = sizeof local;
    if(::getsockname(sockfd,(sockaddr*)&local, &addrLen) < 0) {
        LOG_ERROR("TcpServer::newConnection::getsockname");
    }
    InetAddress localAddr(local);

    // 根据链接成功的sockfd，创建TcpConnection管理生命周期
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallbck(connectionCallbck_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectEstablished,conn)); // 唤醒subLoop执行链接建立操作
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s \n", name_.c_str(),conn->name().c_str());

    connections_.erase(conn->name());
    conn->getLoop()->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}
