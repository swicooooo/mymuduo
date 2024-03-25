#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"

#include <errno.h>
#include <unistd.h>
#include <string.h>

const int KNew = -1;
const int KAdded = 1;
const int KDeleted = 2;

EPollPoller::EPollPoller(EventLoop * loop):Poller(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC)), eventlist_(KInitEventListSize)
{
    if (epollfd_ < 0){
        LOG_FATAL("epoll create error: %d  \n",errno);
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

void EPollPoller::updateChannel(Channel *channel)
{
    int index = channel->index();
    LOG_INFO("fd=%d events=%d index=%d \n",channel->fd(),channel->events(),index);
    // 如果是新添加的，则添加到epollfd_中
    if (index == KNew || index == KDeleted) {
        if(index == KNew) {
            channels_[channel->fd()] = channel;
        }
        channel->setIndex(KAdded);
        update(EPOLL_CTL_ADD, channel);
    }else{
        if(channel->isKNoneEvent()) {
            epoll_ctl(epollfd_, EPOLL_CTL_DEL, channel->fd(),0);
            channel->setIndex(KDeleted);
        }else{
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel *channel)
{
    channels_.erase(channel->fd());
    if(channel->index() == KAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(KNew);
}

void EPollPoller::update(int operation, Channel *channel)
{
    struct epoll_event event;
    memset(&event, 0 ,sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;

    if (::epoll_ctl(epollfd_, operation, channel->fd(),&event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_ERROR("epoll ctl del error: %d \n",errno);
        }
        else{
            LOG_FATAL("epoll ctl add/mod error: %d \n",errno);
        }
    }
}
