#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"

#include <functional>

static EventLoop* checkNoNull(EventLoop *loop)
{
    if(loop == nullptr) {
        LOG_FATAL("%s:%s:%d TcpConnection Loop is nil",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop,const std::string nameArg, int sockfd,const InetAddress &localAddr, const InetAddress &peerAddr)
    : loop_(checkNoNull(loop)), name_(nameArg),state_(KConnecting), reading(true),
    socket_(new Socket(sockfd)),channel_(new Channel(loop,sockfd)),
    localAddr_(localAddr), peerAddr_(peerAddr), highWaterMark_(64*1024*1024)
{
    channel_->setReadEventCallback(std::bind(&TcpConnection::handleRead,this, std::placeholders::_1));
    channel_->setWriteEventCallback(std::bind(&TcpConnection::handleWrite,this));
    channel_->setErrorEventCallback(std::bind(&TcpConnection::handleError,this));
    channel_->setCloseEventCallback(std::bind(&TcpConnection::handleClose,this));

    socket_->setKeepAlive(true);
    LOG_INFO("TcpConnection::ctor[%s] at fd=%d \n", name_.c_str(),sockfd);
}
TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d \n", name_.c_str(),channel_->fd(),(int)state_);
}