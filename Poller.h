#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <vector>
#include <unordered_map>

class Channel;
class EventLoop;

/// @brief 监听当前Loop下的所有channel
class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>; 
    Poller(EventLoop *loop): owner_loop_(loop){}
    virtual ~Poller() = default;

    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannel) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;
    
    bool hasChannel(Channel *channel) const;
    static Poller* newDefaultPoller(EventLoop *loop);
protected:
    using ChannelMap = std::unordered_map<int, Channel*>;   // key: fd, value: Channel*
    ChannelMap channels_;
private:
    EventLoop *owner_loop_;
};