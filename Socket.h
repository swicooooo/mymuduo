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
    // void setReusePort(); ...
private:
    int sockfd_;
};