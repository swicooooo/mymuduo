#include "Acceptor.h"
#include "InetAddress.h"
#include "Logger.h"

#include <cerrno>
#include <functional>
#include <sys/socket.h>

static int createNonBlocking()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if(sockfd < 0) {
        LOG_FATAL("%s:%s:%d listen socketfd create error:%d", __FILE__,__FUNCTION__,__LINE__,errno);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, InetAddress &listenAddr, bool reusePort)
    : listenning_(false), loop_(loop), acceptSock_(createNonBlocking()), acceptChannel_(loop, acceptSock_.fd())
{
    // TODO reuse socket option
    acceptSock_.setReuseAddr(true);
    acceptSock_.setReuseAddr(true);
    acceptSock_.bindAddress(listenAddr);
    acceptChannel_.setReadEventCallback(std::bind(&Acceptor::handleRead, this));
}
Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    listenning_ = true;
    acceptSock_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSock_.accept(&peerAddr);
    if(connfd >= 0) {
        if(newConnCallback_) {
            newConnCallback_(connfd,peerAddr);
        }else {
            LOG_ERROR("%s:%s:%d accept error:%d", __FILE__,__FUNCTION__,__LINE__,errno);            
        }
    }else {
        if(errno == EMFILE) {
            LOG_ERROR("%s:%s:%d sockfd reached limit", __FILE__,__FUNCTION__,__LINE__);  
        }
    }
    
}
