#pragma once

#include "InetAddress.h"

class Socket
{
public:
    explicit Socket(int sockfd): sockfd_(sockfd){}
    ~Socket();

    void bindAddress(InetAddress &localAddr);
    void listen();
    int accept(InetAddress *peerAddr);
    int fd() const{ return sockfd_; }

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on); 
private:
    int sockfd_;
};