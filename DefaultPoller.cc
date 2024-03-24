#include "Poller.h"

#include <stdlib.h>

Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    if (::getenv("MUDUO_USE_POLL"))
        return nullptr;     //返回Poll
    else
        return nullptr;     //返回Epoll
}