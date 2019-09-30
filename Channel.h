#ifndef __NET_CHANNEL_H__
#define __NET_CHANNEL_H__


#include <functional>
#include <sys/epoll.h>
#include "Noncopyable.h"


namespace net
{
class Eventloop;
class Channel: base::noncopyable
{
    public:
    Channel(int fd, Eventloop *loop);
    ~Channel(){printf("\n\nChannel Has Been Cashed!!!\n\n");}
    void enableRead(){event_ |= EPOLLIN; update();}
    void enableWrite(){event_ |= EPOLLOUT; update();}
    void disableRead(){event_ &= ~EPOLLIN; update();}
    void disableWrite(){event_ &= ~EPOLLOUT; update();}
    void disableAll(){event_ = 0; update();}
    //TODO
    //使用异步(加入loop队列)还是同步(线程直接调用)? 是否会有跨线程调用的可能
    void update();
    void handleEvent();
    void setReadCallback(std::function<void()> readCallback){readCallback_ = readCallback;}
    void setWriteCallback(std::function<void()> writeCallback){writeCallback_ = writeCallback;}
    //TODO void setConnection(std::shared_ptr<Connection> ptr){connptr_ = ptr;}
    void setRevent(int event){revent_ = event;}
    int getEvent(){return event_;}
    int getFd(){return fd_;}
    //TODO临时debug
    Eventloop *getLoop(){return loop_;}
    //TODO 
    bool exist;//用于标记channel在epoll中是否已存在

    private:
    int fd_;
    //TODO std::weak_ptr<Connection> connptr_;//使用weak_ptr保存连接
    int event_;//关注事件
    int revent_;//poller返回的活跃事件
    std::function<void()> readCallback_;
    std::function<void()> writeCallback_;
    std::function<void()> errCallback_;
    Eventloop *loop_;//主要用于accepor这样没有连接的类,同时方便更新事件
};  

}

#endif