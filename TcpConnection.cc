#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"

#include <asm-generic/socket.h>
#include <cerrno>
#include <cstdlib>
#include <functional>
#include <sys/socket.h>
#include <muduo/net/Callbacks.h>

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

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int savedErrno = 0;
    int n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if(n > 0) { // fd有数据到达
        messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
    }
    else if(n == 0) {   // 对端关闭
        handleClose();
    }    
    else {
        errno = savedErrno;
        LOG_ERROR("TcpConnection::handleRead \n");
        handleError();
    }

}
void TcpConnection::handleWrite()
{
    if(channel_->isWriting()) {
        int savedErrno = 0;
        int n = outputBuffer_.writeFd(channel_->fd(), &savedErrno);
        if(n > 0) { // 有数据可写
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0) {    // 数据写完
                channel_->disableWriting();
                if(writeCompleteCallbck_) {
                    writeCompleteCallbck_(shared_from_this());
                }
                if(state_ == KDisconnecting) {
                    shutdownInLoop();
                }
            }
        }
        else {
            LOG_ERROR("TcpConnection::handleWrite \n");
        }
    }
    else {
        LOG_ERROR("TcpConnection fd=%d is down, no more writing \n",channel_->fd());
    }

}
void TcpConnection::handleError()
{
    int err;
    int optval = 0;
    socklen_t optlen = sizeof optval;
    if(::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        err = errno;
    }else {
        err = optval;
    }
    LOG_ERROR("TcpConnection handleError name:%s-SO_ERROR:%d \n", name_.c_str(),err);
}
void TcpConnection::handleClose()
{
    LOG_ERROR("TcpConnection handleClose fd=%d state=%d \n", channel_->fd(),(int)state_);
    setState(KDisconnected);
    channel_->disableAll();
    // 处理对端关闭的逻辑
    connectionCallbck_(shared_from_this());
}
