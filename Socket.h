#pragma once

#include "InetAddress.h"

/// @brief 封装sockfd相关的linux的api
class Socket
{
public:
    explicit Socket(int sockfd): sockfd_(sockfd){}
    ~Socket();

    void bindAddress(InetAddress &localAddr);
    void listen();
    int accept(InetAddress *peerAddr);  // 返回通信sockfd
    int fd() const{ return sockfd_; }

    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on); 
private:
    int sockfd_;
};