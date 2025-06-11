
#include <stdlib.h>
#include "EPollPoller.h"

Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        return nullptr;
    }
    else{
        return new EPollPoller(loop); //生成epoll的实例
    }
}