#include "Channel.h"
#include "Eventloop.h"

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
    //TODO 标志?
    if (revent_ & EPOLLIN)
        readCallback_();
    if (revent_ & EPOLLOUT)
        writeCallback_();
    if (revent_ & EPOLLERR)
        errCallback_();
}

void Channel::update()
{
    loop_->update(this);
}