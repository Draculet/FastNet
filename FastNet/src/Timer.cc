#include "../include/Timer.h"
#include "../include/Eventloop.h"
using namespace std;
using namespace net;
using namespace base;

Timer::Timer(Eventloop *loop)
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

Timer::~Timer()
{
    timerChan_->disableAll();
    ::close(timeFd_);
}

int Timer::getTimeFd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        //TODO 日志输出
    }
    return timerfd;
}

uint64_t Timer::readTimeFd()
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
void Timer::addTime(TimeNode &tn)
{
    loop_->runInloop(bind(&Timer::AddTimeInThread, this, tn));
}

void Timer::AddTimeInThread(TimeNode tn)
{
    if (insertMap(tn))
    {
        resetTimer();
    }
}

//跨线程
void Timer::cancelTime(string name)
{
    loop_->runInloop(bind(&Timer::cancelTimeInThread, this, name));
}

void Timer::cancelTimeInThread(string name)
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

void Timer::handleRead()
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


bool Timer::insertMap(TimeNode &tn)
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

void Timer::resetTimer()
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

void Timer::debugPrintMap()
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