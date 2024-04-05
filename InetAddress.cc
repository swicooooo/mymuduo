#include "InetAddress.h"

#include <strings.h>
#include <string.h>
#include <arpa/inet.h>

InetAddress::InetAddress(uint16_t port, std::string ip)
{
    bzero(&addr_,sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(sockaddr_in &addr): addr_(addr){}

std::string InetAddress::toIp() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    return std::string(buf);
}

uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port);
}

std::string InetAddress::toIpPort() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf+strlen(buf),":%u",port);
    return std::string(buf);
}

sockaddr_in *InetAddress::getSockAddr()
{
    return &addr_;
}

