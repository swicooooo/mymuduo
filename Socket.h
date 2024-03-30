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

    // void setReusePort(); ...
private:
    int sockfd_;
};