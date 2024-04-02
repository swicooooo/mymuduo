#include "TcpServer.h"
#include "Logger.h"

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