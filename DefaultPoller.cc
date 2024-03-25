#include "Poller.h"
#include "Channel.h"

#include <stdlib.h>

bool Poller::hasChannel(Channel *channel) const
{
    return channels_.count(channel->fd()) != 0;
}

Poller *Poller::newDefaultPoller(EventLoop *loop)
{
    if (::getenv("MUDUO_USE_POLL"))
        return nullptr;     //返回Poll
    else
        return nullptr;     //返回Epoll
}