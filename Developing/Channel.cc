#include "Channel.h"
#include "Eventloop.h"
#include <assert.h>

using namespace net;
using namespace std;

//fd不一定是socket,不一定需要close,如:listener
Channel::Channel(int fd, Eventloop *loop, Connection *conn)
    :   fd_(fd),
        event_(0),
        revent_(0),
        exist(false),
        loop_(loop),
        conn_(conn)
    {

    }

/*
    11.9新增
    Channel在调用传递来的函数前需要先保护所属Connection
 */

void 
void Channel::handleEvent()
{
    //TODO 其他状态也需要处理
    //TODO 标志?
    //debug 
    //printf("I am In Channel::handleEvent\n");
    shared_ptr<Connection> conn;
    //下列函数都含有connection的裸指针,调用前需要先绑定所属Connection
    //一则保护Connection调用期间安全
    //二则防止Connection调用前以析构
    conn = conn_.lock();
    if (conn)
    {
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
    }
    //debug
    //printf("I am Leaving Channel::handleEvent\n");
}

void Channel::update()
{
    loop_->update(this);
}