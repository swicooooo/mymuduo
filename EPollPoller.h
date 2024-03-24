#pragma once

#include "Poller.h"
#include "TimeStamp.h"

#include <sys/epoll.h>
#include <vector>

class EventLoop;
class Channel;

/**
 * EPoll
 * epoll_create     epoll_ctl    epoll_wait
*/
class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;

    TimeStamp poll(int timeoutMs, ChannelLists *activeChannel) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

private:
    void fillActiveChannel(int numEvents, ChannelLists *activeChannel) const; // 填充活跃的通道
    void update(int operation, Channel *channel); // 更新通道状态

    static const int KInitEventListSize = 16;
    using EventList = std::vector<epoll_event>;
    int epoll_fd_;
    EventList event_list_;
};