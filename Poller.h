#pragma once 
#include "noncopyable.h"
#include <vector>
#include <unordered_map>
#include "Timestamp.h"


class Channel;
class EventLoop;


class Poller
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    //给所有IO复用保留统一的接口
    virtual Timestamp poll(int timesoutMs,ChannelList *activeChannel) = 0;
    virtual void updateChannel(Channel *Channel) = 0;
    virtual void removeChannel(Channel *Channel) = 0;

    //判断参数channel是否在当前Poller当中
    bool hasChannel(Channel *channel) const;

    //EventLoop可以通过该接口获取默认的IO复用的具体实现
    static Poller* newDefaultPoller(EventLoop *loop);

protected:
    //map的key：sockfd  value: sockfd所属的channel通道类型
    using ChannelMap = std::unordered_map<int,Channel*>;
    ChannelMap channels_;


private:
    EventLoop *ownerLoop_;//定义Poller所属的事件循环EventLoop

};