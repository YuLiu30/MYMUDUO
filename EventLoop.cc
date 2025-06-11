#include "EventLoop.h"
#include "Logger.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "CurrentThread.h"

#include "Channel.h"


//防止一个线程创建多个EventLoop
__thread EventLoop *t_loopInThisThread = nullptr;

//定义默认的Poller IO复用接口超时时间
const int kPollTimesMs = 10000;

//创建wakeupfd，用来notify唤醒subReactor处理新来的channel
int createEventfd()
{
    int evtfd = ::eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0)
    {
        LOG_FATAL("eventfd error : %d \n",errno);
    }
    return evtfd;
}

EventLoop::EventLoop():
    looping_(false),
    quit_(false),
    callingPendingFunctors_(false),
    threadId_(CurrentThread::tid()),
    poller_(Poller::newDefaultPoller(this)),
    wakeupFd_(createEventfd()),
    wakeupChanner_(new Channel(this,wakeupFd_))
{
    LOG_DEBUG("EventLoop created %p in thread %d \n",this,threadId_);
    if(t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d \n",t_loopInThisThread,threadId_);
    }
    else{
        t_loopInThisThread = this;
    }
    //设置wakeupfd事件类型以及发生事件后的回调操作
    wakeupChanner_->setReadCallback(std::bind(&EventLoop::handleRead,this));
    // 每一个eventloop都将监听wakeupchannel的EPOLLIN读事件了
    wakeupChanner_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChanner_->disableAll();
    wakeupChanner_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

//开启事件循环
void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    LOG_INFO("EventLoop %p start looping \n",this);

    while(!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimesMs,&activeChannels_);
        for(Channel *channel : activeChannels_)
        {
            channel->handleEvent(pollReturnTime_);
        }

        doPendingFunctors();
    }
    LOG_INFO("EventLoop %p stop looping. \n",this);
    looping_ = false;
}

//退出事件循环
//1.loop在自己的线程中调用quit
//2.在非loop的线程中，调用loop的quit
void EventLoop::quit()
{
    quit_ = true;
    //如果是在其它线程中，调用的quit  在一个subloop(woker)中，调用了mainLoop(I0)的quit
    if(!isInLoopThread())
    {
        wakeup();
    }
}

    //在当前loop中执行cb
void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread())// 在当前loop线程中执行cb
    {
        cb();
    }
    else{
        queueInLoop(cb); //在当前非loop线程中执行cb，需要唤醒loop所在线程，执行cb
    }
}

//  把cb放入队列中，唤醒loop所在线程，执行cb
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    //唤醒相应的需要执行上面回调操作的loop线程
    //|| callingPendingFunctors_的意思是：当前loop正在执行回调，但是loop又有了新的回调
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();//唤醒loop所在线程
    }
}


void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_,&one,sizeof one);
    if(n!=sizeof one)
    {
        LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8",n);
    }
}

//唤醒loop所在的线程
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_,&one,sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n",n);
    }
}

//EventLoop方法 =》 Poller的方法
void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors() //执行回调
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(const Functor &functor:functors)
    {
        functor();
    }
    callingPendingFunctors_ = false;
}