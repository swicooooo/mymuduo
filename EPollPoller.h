#pragma once

#include "Poller.h"
#include "Timestamp.h"

#include <sys/epoll.h>
#include <vector>

class EventLoop;
class Channel;

/**
 * EPoller通过三个状态控制Channel的注册和销毁
 * epoll_create     epoll_ctl    epoll_wait
*/
class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList *activeChannel) override;
    void updateChannel(Channel *channel) override;      // 在Channel中改变event状态后poller相应改变epoll中的fd感兴趣事件
    void removeChannel(Channel *channel) override;      // 从epoll和Channels中移除channel
private:
    void update(int operation, Channel *channel);       // 更新fd在epoll中的感兴趣事件
    void fillActiveChannel(int numEvents, ChannelList *activeChannel) const; // 填充活跃的Channel

    static const int KInitEventListSize = 16;
    using EventList = std::vector<epoll_event>;
    int epollfd_;          // poller的标识符
    EventList eventlist_;  // poller保存的事件合集
};