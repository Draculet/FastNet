#ifndef __TIMER_H__
#define __TIMER_H__

#include "UnixTime.h"
#include "Channel.h"
#include "Eventloop.h"
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

using namespace std;
using namespace net;

class TimeNode
{
    public:
    TimeNode(function<void()> callback, UnixTime timeout, double round, string name)
        :callback_(callback),
        timeout_(timeout),
        round_(round),
        name_(name)
    {

    }

    TimeNode()
        :callback_(),
        timeout_(UnixTime(0)),
        round_(0),
        name_()
    {

    }

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

    string getName() const
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
    string name_;
    function<void()> callback_;
};

bool operator<(TimeNode tn1, TimeNode tn2)
{
    return tn1.getName() < tn2.getName();
}



//Timer所有方法都需要处理为同步调用
/*
    名字指定唯一计时器
*/
class Timer
{
    public:
    Timer(Eventloop *loop)
        :loop_(loop),
        timeFd_(getTimeFd()),
        timerChan_(new Channel(timeFd_, loop_)),
        timeMap_(),
        times_(),
        timeouts_()
    {
        timerChan_->setReadCallback(bind(&Timer::handleRead, this));
        timerChan_->enableRead();
    }

    ~Timer()
    {
        timerChan_->disableAll();
        ::close(timeFd_);
    }

    int getTimeFd()
    {
        int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        if (timerfd < 0)
        {
            //TODO 日志输出
        }
        return timerfd;
    }

    uint64_t readTimeFd()
    {
        uint64_t count;
        size_t n = ::read(timeFd_, &count, sizeof(count));
        if (n != sizeof(count))
        {
            assert(n == sizeof(count));
            //TODO 日志报错
        }
        return count;  
    }

    //跨线程调用,使用runInloop
    void addTime(TimeNode &tn)
    {
        loop_->runInloop(bind(&Timer::AddTimeInThread, this, tn));
    }

    void AddTimeInThread(TimeNode tn)
    {
        if (insertMap(tn))
        {
            resetTimer();
        }
    }

    //跨线程
    void cancelTime(string name)
    {
        loop_->runInloop(bind(&Timer::cancelTimeInThread, this, name));
    }

    void cancelTimeInThread(string name)
    {
        auto it = times_.find(name);
        if (it != times_.end())
        {
            UnixTime t = it->second.getTime();
            auto iter = timeMap_[t].find(it->second);
            if (iter != timeMap_[t].end())
            {
                timeMap_[t].erase(iter);
                //注意当set空时要将set删去
                if (timeMap_[t].empty())
                {
                    timeMap_.erase(t);
                }
                times_.erase(it);
            }
            else
            {
                //TODO 日志输出
                printf("*debug* Error In CancelTime\n");
            }
        }
        else
        {
            //TODO 日志输出
            printf("*debug* Time Not Found\n");
        }
    }

    void handleRead()
    {
        printf("*debug* Timer::handleRead()\n");
        readTimeFd();
        UnixTime now(UnixTime::now());
        auto itend = timeMap_.lower_bound(now);
        if (itend == timeMap_.begin())
        {
            printf("*debug* No Time Timeout\n");
        }
        for (auto iter = timeMap_.begin(); iter != itend; iter++)
        {
            for (auto &obj : iter->second)
            {
                timeouts_.push_back(obj);
            }
        }
        printf("timeouts size: %ld\n", timeouts_.size());
        for (auto &obj : timeouts_)
        {
            times_.erase(obj.getName());
        }
        timeMap_.erase(timeMap_.begin(), itend);
        for (auto &obj : timeouts_)
        {
            obj.cb();
            if (obj.round())
            {
                obj.update();
                insertMap(obj);
            }
        }
        //cout << "in handleRead()" << endl;
        //printMap();
        timeouts_.clear();
        resetTimer();
    }


    bool insertMap(TimeNode &tn)
    {
        bool reset = false;
        UnixTime t = tn.getTime();
        string name = tn.getName();
        auto it = timeMap_.begin();
        if (it == timeMap_.end() || t < it->first)
        {
            printf("*debug* Need Reset\n");
            reset = true;
        }
        else
        {
            printf("*debug* No Need Reset\n");
        }
        auto iter = times_.find(tn.getName());
        if (iter != times_.end())
        {
            cancelTimeInThread(name);
            times_[name] = tn;
            timeMap_[t].insert(tn);
        }
        else
        {
            times_[name] = tn;
            timeMap_[t].insert(tn);
        }
        /* 
        string tname = tn.getName();
        auto iter = timeMap_.find(t);
        if (iter != timeMap_.end())
        {
            auto iter = timeMap_[t].find(tn);
            if (iter != timeMap_[t].end())
            {
                
                    //注意set中的元素默认const,是不能任意修改的,修改会造成排序失效
                
            }
            else //iter == timeMap_[t].end()
            {
                timeMap_[t].insert(tn);
            }
        }
        else //iter == timeMap_.end()
        {
            timeMap_[t].insert(tn);
        }
        */
        return reset;
    }

    void resetTimer()
    {
        UnixTime latest;
        auto iter = timeMap_.begin();
        if (iter != timeMap_.end())
        {
            latest = iter->first;
            UnixTime now = UnixTime::now();
            int64_t us = latest.getUsFromEpoch() - now.getUsFromEpoch();
            printf("*debug* resetTimer: time: %ld\n", us);
            if (us < 1000)//处理延时要求大于1ms 1ms = 1000us
            {
                us = 1000;
            }
            struct timespec ts;
            ts.tv_sec = static_cast<time_t>(us / UnixTime::million);
            ts.tv_nsec = (us % UnixTime::million) * 1000;
            struct itimerspec tim;
            struct itimerspec tim2;
            ::memset(&tim, 0, sizeof(tim));
            ::memset(&tim2, 0, sizeof(tim2));
            tim.it_value = ts;
            int res = ::timerfd_settime(timeFd_, 0, &tim, &tim2);
            if (res)
            {
                //TODO 日志输出
                assert(res > 0);
            }
        }
        else
        {
            //TODO 日志输出
            printf("*debug* No Timer\n");
        }
    }

    void debugPrintMap()
    {
        for (auto &p : timeMap_)
        {
            cout << "UnixTime: " << p.first.toLocString() << endl;
            for (auto &obj : p.second)
            {
                cout << "Timeout: "  << obj.getTime().toLocString() << endl;
                cout << "Name: " << obj.getName() << endl;
            }
        }
        cout << endl;
        cout << "TimeOut Queue: " << endl;
        for (auto &obj : timeouts_)
        {
            cout << "Timeout: "  << obj.getTime().toLocString() << endl;
            cout << "Name: " << obj.getName() << endl;
        }
        cout << endl;
        cout << "Times Queue: " << endl;
        for (auto &obj : times_)
        {
            cout << "TimeName: " << obj.first << endl;
        }
    }

    private:
    Eventloop *loop_;
    int timeFd_;
    unique_ptr<Channel> timerChan_;
    map< UnixTime, set<TimeNode> > timeMap_;
    map< string, TimeNode> times_;
    vector<TimeNode> timeouts_;
};

#endif