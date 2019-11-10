#include "Poller.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "Channel.h"

using namespace std;
using namespace net;
using namespace base;

Poller::Poller():
    epfd_(epoll_create1(EPOLL_CLOEXEC)),
    events_(kinitlist)
{

}

int Poller::poll(channelList *activelist)
{
    //TODO 不超时返回?
    //TODO events_伸缩
    assert(events_.size() >= kinitlist);
    int ret = epoll_wait(epfd_, &*events_.begin(), events_.size(), -1);
    printf("epoll_wait ret %d\n", ret);
    if (ret == -1)
        perror("epoll");
    if (ret > 0)
    {
        fillActiveChannel(activelist, ret);
        //弹性增长
        if (ret == events_.size())
        {
            events_.resize(ret * 2);
        }
    }
    return ret;
}

void Poller::fillActiveChannel(channelList *activelist, int ret)
{  
    assert(ret > 0);
    for (int i = 0; i < ret; i++)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        assert(channel != nullptr);
        channel->setRevent(events_[i].events);
        //TODO 是否可以优化?
        activelist->push_back(channel);
    }
    //将vector清零而不能用clear,否则size == 0,epoll_wait会报错
    //清零是否必要?
    /*
    for (int i = 0; i < ret; i++)
    {
        memset(&events_[i], 0, sizeof(events_[i]));
    }
    */
}

void Poller::update(Channel *channel)
{
    epoll_event ev;
    ev.events = channel->getEvent();
    ev.data.ptr = channel;
    if (channel->exist)
    {
        if (ev.events == 0)
        {
            printf("removechannel %d\n", channel->getFd());
            removeChannel(channel->getFd());
            channel->exist = false;
            return;//FIXME 多个出口
        }
        else
        {
            printf("modchannel %d\n", channel->getFd());
            //不用担心传入的ev是指针epoll_event有生命周期问题,因为epoll会在内核拷贝这个结构体
            //需要保证的是channel的生命周期
            int ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, channel->getFd(), &ev);
            assert(ret != -1);
            //TODO epoll_ctl返回错误处理
        }
    }
    else
    {
        if (ev.events != 0)//events == 0 && ev.exist == false什么都不做
        {
            printf("addchannel %d\n", channel->getFd());
            int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, channel->getFd(), &ev);
            assert(ret != -1);
            channel->exist = true;
        }
    }
}

void Poller::removeChannel(int fd)
{
    //TODO 保证fd在channel map里存在 
    //if ()
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
    assert(ret != -1);//debug 出现过问题
    //TODO epoll_ctl返回错误处理
}