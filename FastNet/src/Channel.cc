#include "../include/Channel.h"
#include "../include/Eventloop.h"
#include <assert.h>
#include "../include/Gettid.h"

using namespace base;
using namespace net;
using namespace std;

//fd不一定是socket,不一定需要close,如:listener
Channel::Channel(int fd, Eventloop *loop)
    :   fd_(fd),
        event_(0),
        revent_(0),
        exist(false),
        loop_(loop),
        binded_(false)
{

}

Channel::~Channel()
{
    printf("*debug* ~Channel()\n");
}
/*
    11.9新增
    Channel在调用传递来的函数前需要先保护所属Connection
 */

void Channel::bindConn(shared_ptr<Connection> conn)
{
    conn_ = conn;
    binded_ = true;
}

/*
    readCallback_等callback只传Connection的裸指针,传shared_ptr<Connection>会影响Connection析构
    通过conn_.lock()保护指针
*/
void Channel::handleEvent()
{
    //printf("*debug* handleEvent\n");
    //printf("*debug* Attention ~Connection fd %d\n", fd_);
    //针对acceptor这类不需要绑定Connection的Channel
    if (!binded_)
    {
        if (revent_ & EPOLLIN)
        {
            //printf("*debug* EPOLLIN\n");
            if (readCallback_)
                readCallback_();
        }
        //printf("*debug* Attention ~Connection Thread %d\n", fd_);
        if (revent_ & EPOLLOUT)
        {
            //printf("*debug* EPOLLOUT\n");
            if (writeCallback_)
                writeCallback_();
        }
        //printf("*debug* Attention ~Connection Thread %d\n", fd_);
        if (revent_ & EPOLLERR)
        {
            //printf("*debug* EPOLLERR\n");
            if (errCallback_)
                errCallback_();
        }
        //printf("*debug* Attention ~Connection Thread %d\n", fd_);
    }
    else
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
    }
    //debug
    //printf("I am Leaving Channel::handleEvent\n");
}

void Channel::update()
{
    loop_->update(this);
}