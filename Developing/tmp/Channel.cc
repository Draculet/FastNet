#include "Channel.h"
#include "Eventloop.h"
#include <assert.h>

using namespace net;
using namespace std;

//fd不一定是socket,不一定需要close,如:listener
Channel::Channel(int fd, Eventloop *loop)
    :   fd_(fd),
        event_(0),
        revent_(0),
        exist(false),
        loop_(loop)

    {

    }

void Channel::handleEvent()
{
    //TODO 其他状态也需要处理
    //TODO closeCallback()没有调用
    //TODO 标志?
    //debug 
    //printf("I am In Channel::handleEvent\n");
    if (revent_ & EPOLLIN)
    {
        if (readCallback_)
            readCallback_();
    }
    if (revent_ & EPOLLOUT)
    {
        if (writeCallback_)
            writeCallback_();
    }
    if (revent_ & EPOLLERR)
    {
        if (errCallback_)
            errCallback_();
    }
    //debug
    //printf("I am Leaving Channel::handleEvent\n");
}

void Channel::update()
{
    loop_->update(this);
}