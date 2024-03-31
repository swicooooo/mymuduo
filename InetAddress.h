#pragma once

#include <netinet/in.h>
#include <string>

class InetAddress
{
public:
    explicit InetAddress(uint16_t port, std::string i = "127.0.0.1");
    explicit InetAddress(struct sockaddr_in &addr);
    InetAddress() = default;

    std::string toIp() const; // 获取Ip
    uint16_t toPort() const;  // 获取port
    std::string toIpPort() const; // 获取IpPort

    struct sockaddr_in* getSockAddr(); // 获取Sockaddr
    void setSockAddr(sockaddr_in addr) { addr_ = addr; }
private:
    struct sockaddr_in addr_;
};