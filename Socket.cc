#include "Socket.h"
#include "Logger.h"

#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Socket::~Socket()
{
    ::close(sockfd_);
}
void Socket::bindAddress(InetAddress &localAddr)
{
    if(::bind(sockfd_, (sockaddr*)localAddr.getSockAddr(), sizeof(sockaddr_in)) != 0){
        LOG_FATAL("sockfd bind error: %d \n",sockfd_);
    }
}
void Socket::listen()
{
    if(::listen(sockfd_, 1024) != 0) {
        LOG_FATAL("sockfd listen error: %d \n",sockfd_);
    }
}
int Socket::accept(InetAddress *peerAddr)
{
    struct sockaddr_in addr;
    socklen_t len;
    ::memset(&addr, 0, sizeof addr);
    int connfd = ::accept(sockfd_, (sockaddr*)&addr, &len);
    if(connfd >= 0){
        peerAddr->setSockAddr(addr);
    }
    return connfd;
}