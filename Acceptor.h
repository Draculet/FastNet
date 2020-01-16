#ifndef __ACCEPETOR_H__
#define __ACCEPETOR_H__

#include "Eventloop.h"
#include "Socket.h"
#include "Channel.h"
#include "NetAddr.h"
#include <errno.h>

using namespace net;
using namespace std;
class Acceptor : base::noncopyable
{
    public:
    Acceptor(Eventloop *loop, int port):
        acceptloop_(loop),
        acceptsoc_(new Socket(port)),   //socket默认setReuse,setNonBlock
        acceptchan_(new Channel(acceptsoc_->getFd(), loop)),
        extraFd_(open("/dev/null", O_RDWR))
    {
        
    }

    ~Acceptor()
    {
        acceptchan_->disableAll();
        ::close(extraFd_);
    }

    //DEBUG用
    int getAcceptFd()
    {
        return acceptsoc_->getFd();
    }

    void setNewConnCallback(function<void (int, NetAddr&)> newConnCallback)
    {
        newConnCallback_ = newConnCallback;
    }
    
    void listen()
    {
        acceptchan_->setReadCallback(bind(&Acceptor::handleRead, this));
        acceptsoc_->listen();
        printf("*debug* accept channel add\n");
        acceptchan_->enableRead();
    }

    void handleRead()
    {
        NetAddr addr;
        int sockfd = acceptsoc_->accept(&addr);
        printf("*debug* accept conn %s\n", addr.toString().c_str());
        if (sockfd < 0)//返回-1,可能是异步正常返回,也可能是fd用完
        {
            if (errno == EMFILE)
            {
                //TODO 日志输出
                NetAddr tmp;
                ::close(extraFd_);
                extraFd_ = acceptsoc_->accept(&tmp);
                ::close(extraFd_);
                extraFd_ = ::open("/dev/null", O_RDWR);
            }
        }
        else//当标准输入关闭时,sockfd可能返回0
        {
            if (newConnCallback_)
                newConnCallback_(sockfd, addr);
            else//出现错误
            {
                printf("*debug* no newConnection\n");
                ::close(sockfd);
            }
        }
    }

    private:
    Eventloop *acceptloop_;
    unique_ptr<Socket> acceptsoc_;
    unique_ptr<Channel> acceptchan_;
    int extraFd_;//防止fd用完无法关闭新到连接
    
    function<void (int, NetAddr&)> newConnCallback_;
};

#endif