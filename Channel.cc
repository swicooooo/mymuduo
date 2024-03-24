#include "Channel.h"

#include <sys/epoll.h>

const int Channel::KNoneEvent = 0;
const int Channel::KReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::KWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd) :loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1){}

void Channel::handleEvent(TimeStamp receiveTime)
{
    if (tied_) {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard) {
            handleEventWithGuard(receiveTime);
        }
    }else {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::remove()
{
    // TODO
    // loop_.removeChannel(this);
}

void Channel::update()
{
    // TODO
    // loop_.updateChannel(this);
}

void Channel::handleEventWithGuard(TimeStamp receiveTime)
{
    if (revents_ & EPOLLHUP && !(revents_ & EPOLLIN)) {
        if (closeEventCallback_) closeEventCallback_();
    }

    if (revents_ & EPOLLERR) {
        if (errorEventCallback_) errorEventCallback_();
    }

    if (revents_ & EPOLLIN | revents_ & EPOLLPRI) {
        if (readEventCallback_) readEventCallback_(receiveTime);
    }

    if (revents_ & EPOLLOUT) {
        if (writeEventCallback_) writeEventCallback_();
    }
}

