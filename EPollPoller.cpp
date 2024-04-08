#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"
#include "Timestamp.h"

#include <asm-generic/errno-base.h>
#include <errno.h>
#include <sys/epoll.h>
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

// 使用epoll_wait等待事件发生，并且填充activeChannel
Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannel)
{
    // LOG_DEBUG 
    LOG_INFO("func: %s => fd total size: %lu \n",__FUNCTION__, activeChannel->size());
    Timestamp timestamp(Timestamp::now());
    int saveErrno = errno;
    int numEvents=::epoll_wait(epollfd_, &*eventlist_.begin(), sizeof eventlist_, timeoutMs);
    if(numEvents > 0) {
        LOG_INFO("func: %s => event happen: %d \n",__FUNCTION__, numEvents);    
        fillActiveChannel(numEvents, activeChannel);
        if(numEvents == eventlist_.size())
            eventlist_.resize(numEvents*2);
    }
    else if(numEvents == 0) {
        LOG_INFO("func: %s => timeout! \n",__FUNCTION__);
    }
    else if(numEvents < 0) {
        if(saveErrno != EINTR) {
            errno = saveErrno;
            LOG_INFO("func: %s => poll error %d \n",__FUNCTION__, errno);
        }
    }
    return timestamp;
}

void EPollPoller::fillActiveChannel(int numEvents, ChannelList *activeChannel) const
{
    for(int i=0; i<numEvents; ++i) {
        Channel *channel = static_cast<Channel*>(eventlist_[i].data.ptr);
        channel->setRevent(eventlist_[i].events);
        activeChannel->push_back(channel);
    }
}

void EPollPoller::updateChannel(Channel *channel)
{
    int index = channel->index();
    LOG_INFO("func: %s => fd=%d events=%d index=%d \n",__FUNCTION__,channel->fd(),channel->events(),index);
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
    LOG_INFO("func: %s => fd=%d \n",__FUNCTION__,channel->fd());
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
