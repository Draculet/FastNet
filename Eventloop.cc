#include <stdio.h>
#include "Eventloop.h"
#include "Gettid.h"
#include <assert.h>
#include "Channel.h"

using namespace std;
using namespace net;
using namespace base;

Eventloop::Eventloop()
    : poller_(new Poller()),
    quit_(false)
{

}

void Eventloop::loop()
{
    assert(quit_ == false);
    while (!quit_)
    {
        //poll返回值处理?
        int ret = poller_->poll(&activelist);
        if (ret == 0)
            ;//TODO 报告无事件发生
        else if (ret == -1)
            ;//TODO 报告错误发生
        else if (activelist.size() != 0)
        {
            for (auto &channel : activelist)
            {
                channel->handleEvent();
            }
            activelist.clear();
        }
        dotasks();
    }
    //TODO 
}

void Eventloop::runInThread()
{
    {
        MutexGuard mutex(mutex_);
        tid_ = base::gettid();
    }
    loop();
}

void Eventloop::dotasks()
{
    taskQueue tmptasks;
    {
        MutexGuard mutex(mutex_);
        tmptasks.swap(tasks);
    }
    for (auto task : tmptasks)
    {
        task();
    }
}

void Eventloop::insertQueue(function<void()> func)
{
    {
        MutexGuard mutex(mutex_);
        tasks.push_back(std::move(func));
    }
    //TODO wakeup();
}

void Eventloop::runInloop(function<void()> func)
{
    if (inloop())
        func();
    else
        insertQueue(std::move(func));//调用移动构造函数
}

void Eventloop::update(Channel *channel)
{
    poller_->update(channel);
}