#include "Poller.h"
#include "Channel.h"
#include "EPollPoller.h"

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
        return new EPollPoller(loop);     //返回EpollPoller
}