#ifndef __TIMER_H__
#define __TIMER_H__

#include "UnixTime.h"
#include "Channel.h"
#include <vector>
#include <functional>
#include <map>
#include <set>
#include <unistd.h>
#include <memory>
#include <sys/timerfd.h>
#include <assert.h>
#include <cstring>
#include <iostream>

namespace net
{
class TimeNode
{
    public:
    TimeNode(std::function<void()> callback,  UnixTime timeout, double round, std::string name)
        :callback_(callback),
        timeout_(timeout),
        round_(round),
        name_(name)
    {}

    TimeNode()
        :callback_(),
        timeout_( UnixTime(0)),
        round_(0),
        name_()
    {}

    bool round()
    {
        return round_ > 0.0;
    }

    void cb()
    {
        callback_();
    }

    void update()
    {
        if (round_ > 0.0)
        {
            timeout_ += round_;
        }
    }

    std::string getName() const
    {
        return name_;
    }

     UnixTime getTime() const
    {
        return timeout_;
    }
    
    private:
    UnixTime timeout_;
    double round_;
    std::string name_;
    std::function<void()> callback_;
};

//需要内联,否则编译会出现multi define错误
inline bool operator<( TimeNode tn1,  TimeNode tn2)
{
    return tn1.getName() < tn2.getName();
}

}


//Timer所有方法都需要处理为同步调用
/*
    名字指定唯一计时器
*/
namespace net
{
class Eventloop;
class Timer
{
    public:
    Timer( Eventloop *loop);
    ~Timer();
    int getTimeFd();
    uint64_t readTimeFd();
    //跨线程调用,使用runInloop
    void addTime( TimeNode &tn);
    void AddTimeInThread( TimeNode tn);
    //跨线程
    void cancelTime(std::string name);
    void cancelTimeInThread(std::string name);
    void handleRead();
    bool insertMap( TimeNode &tn);
    void resetTimer();
    void debugPrintMap();

    private:
     Eventloop *loop_;
    int timeFd_;
    std::unique_ptr< Channel> timerChan_;
    std::map<  UnixTime, std::set< TimeNode> > timeMap_;
    std::map< std::string,  TimeNode> times_;
    std::vector< TimeNode> timeouts_;
};
}
#endif