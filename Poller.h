#pragma once

#include "noncopyable.h"
#include "TimeStamp.h"

#include <vector>
#include <unordered_map>

class Channel;
class EventLoop;

/** 
 *  Demultiplex : IO复用模块
*/
class Poller : noncopyable
{
public:
    using ChannelLists = std::vector<Channel*>; 
    Poller(EventLoop *loop): owner_loop_(loop){}
    virtual ~Poller() = default;

    virtual TimeStamp poll(int timeoutMs, ChannelLists *activeChannel) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;
    
    bool hasChannel(Channel *channel) const{ return channelMap.count(channel->fd()) != 0; }
    static Poller* newDefaultPoller(EventLoop *loop);
protected:
    using ChannelMap = std::unordered_map<int, Channel*>;   // key: fd, value: Channel*
    ChannelMap channelMap;
private:
    EventLoop *owner_loop_;
};