#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"
#include <unistd.h>
#include <errno.h>
#include <strings.h>

//channel未添加到poller中
const int kNew = -1;    //channel的成员index_ = -1
//channe已添加到poller中
const int kAdded = 1;
//channel从poller中删除
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop *loop)
    :Poller(loop)
    ,epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    ,events_(kInitEventListSize)
{
    if(epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error :%d\n",errno);
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}


//重写基类POller的抽象方法
Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    LOG_INFO("func=%s => fd total count:%lu \n",__FUNCTION__,channels_.size());

    int numEvents = ::epoll_wait(epollfd_,&*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());

    if(numEvents > 0 )
    {
        LOG_INFO("%d events happened \n",numEvents);
        fillActiveChannels(numEvents,activeChannels);
        if(numEvents == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if(numEvents == 0)
    {
        LOG_DEBUG("%s timeout! \n", __FUNCTION__);
    }
    else
    {
        if(saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_ERROR("EPollPoller::poll() err!");
        }
    }
    return now;
}

void EPollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();
    LOG_INFO("func=%s => fd = %d   events = %d index = %d \n",__FUNCTION__,channel->fd(),channel->events(),index);
    if(index == kNew || index == kDeleted)
    {
        if(index == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel;
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD,channel);
    }
    else{
        int fd = channel->fd();
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL,channel);
            channel->set_index(kDeleted);
        }
        else{
            update(EPOLL_CTL_MOD,channel);
        }
    }
}

void EPollPoller::removeChannel(Channel *channel) 
{
    int fd = channel->fd();
    channels_.erase(fd);

    LOG_INFO("func=%s => fd = %d",__FUNCTION__,fd);
    int index = channel->index();
    if(index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
    
}

//填写活跃的channel
void EPollPoller::fillActiveChannels(int numEvents,ChannelList *activeChannels) const
{
    for(int i=0;i<numEvents;++i)
    {
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

//更新channel通道
void EPollPoller::update(int operation ,Channel *channel)
{
    epoll_event event;
    bzero(&event,sizeof event);

    int fd = channel->fd();
    event.events = channel->events();
    event.data.ptr = channel;
    event.data.fd=fd;
    

    if(::epoll_ctl(epollfd_,operation,fd,&event)<0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n",errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl del error:%d\n",errno);
        }
    }
}
