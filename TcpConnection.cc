#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

#include <asm-generic/socket.h>
#include <cerrno>
#include <cstdlib>
#include <functional>
#include <sys/socket.h>

static EventLoop* checkNoNull(EventLoop *loop)
{
    if(loop == nullptr) {
        LOG_FATAL("%s:%s:%d TcpConnection Loop is nil",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop,const std::string nameArg, int sockfd,const InetAddress &localAddr, const InetAddress &peerAddr)
    : loop_(checkNoNull(loop)), 
        name_(nameArg),state_(KConnecting), 
        reading(true),
        socket_(new Socket(sockfd)),
        channel_(new Channel(loop,sockfd)),
        localAddr_(localAddr), 
        peerAddr_(peerAddr), 
        highWaterMark_(64*1024*1024)
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

void TcpConnection::send(const std::string &buf)
{
    if(state_ == KConnected) {
        if(loop_->isInLoopThread()) {
            sendInLoop(buf.c_str(), buf.size());
        }else{
            loop_->queueInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}

void TcpConnection::shutdown()
{
    if(state_ == KConnected) {
        setState(KDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::connectEstablished()
{
    setState(KConnected);
    channel_->tie(shared_from_this()); // 弱指针持有TcpConnection对象，防止调用时未知错误
    channel_->enableReading();

    connectionCallbck_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    if(state_ == KConnected) {
        setState(KDisconnected);
        channel_->disableAll();

        connectionCallbck_(shared_from_this());
    }
    channel_->remove(); // 从poller中删除channel
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
                if(writeCompleteCallback_) {
                    writeCompleteCallback_(shared_from_this());
                }
                if(state_ == KDisconnecting) {  // 如果调用了shutdown方法，则写完就调用shutdownInLoop
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
    closeCallback_(shared_from_this());
}

// 如果缓冲区没有数据，则发送新数据，如果有则先发送缓冲区数据
void TcpConnection::sendInLoop(const char *msg, int len)
{
    ssize_t nwrote =0;
    size_t remaining = len;
    bool faultError = false;

    if(state_ == KDisconnected) {
        LOG_ERROR("disconnected, give up writing!");
    }
    // channel第一次写数据，并且缓冲区没有待发送数据
    if(!channel_->isWriting() && outputBuffer_.readableBytes()==0) {
        nwrote = ::write(channel_->fd(), msg, len);

        if(nwrote >= 0) {
            remaining = len - nwrote;
            if(remaining == 0 && writeCompleteCallback_) {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }else{
            nwrote = 0;
            if(errno != EWOULDBLOCK) {  // 阻塞返回
                LOG_ERROR("TcpConnection::sendInLoop");
                if(errno != EPIPE || errno != ECONNRESET) { // 传输出错
                    faultError = true;
                }
            }
        }
    }

    // 数据未发送完全，剩余的数据存入outputBuffer,并且通知poller注册epollout事件，来持续监听是否发送完全
    if(!faultError && remaining>0) {
        size_t oldLen = outputBuffer_.readableBytes();
        // 如果oldLen没有触发高水位线回调，则调用它
        if(oldLen+remaining > highWaterMark_ && oldLen<highWaterMark_ && highWaterMarkCallback_) {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen+remaining));
        }

        outputBuffer_.append(msg+nwrote, remaining);
        if(!channel_->isWriting()) {
            // 注册channel写事件,poller更新channel状态后，channel会调用写setWriteEventCallback，
            // 即TcpConnection::handleWrite发送outputBuffer剩余的数据
            channel_->enableWriting();  
        }
    }
}

void TcpConnection::shutdownInLoop()
{
    if(!channel_->isWriting()) {    // outputBuffer中的数据发送完成
        socket_->shutdownWrite();
    }
}
