#pragma once

#include "Timestamp.h"
#include "Poller.h"
#include <vector>
#include <sys/epoll.h>
#include "Channel.h"

class EPollPoller :public Poller
{
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;
    
    //重写基类POller的抽象方法
    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize = 16;
    //填写活跃的channel
    void fillActiveChannels(int numEvents,ChannelList *activeChannels) const;
    //更新channel通道
    void update(int operation ,Channel *channel);

    using EventList = std::vector<epoll_event>;

    int epollfd_;
    EventList events_;
};