#include <stdio.h>
#include "Eventloop.h"
#include "Gettid.h"
#include <assert.h>
#include "Channel.h"
#include <sys/eventfd.h>
#include "Gettid.h"

using namespace std;
using namespace net;
using namespace base;

Eventloop::Eventloop():
    poller_(new Poller()),
    tid_(gettid()), //初始化时直接赋值
    quit_(false),
    wakeFd_(getWakeupFd()),
    wakechan_(new Channel(wakeFd_, this)),
    mutex_()
{
    wakechan_->setReadCallback(bind(&Eventloop::handleRead, this));
    printf("*debug* wakeupchannel add\n");
    wakechan_->enableRead();
}

Eventloop::~Eventloop()
{
    wakechan_->disableAll();
    ::close(wakeFd_);
}

void Eventloop::handleRead()
{
    printf("*debug* In Thread %d Be WakeUped()\n", gettid());
    uint64_t n = 1;
    int ret = read(wakeFd_, &n, sizeof(uint64_t));//保证将缓冲区清空
    if (ret != sizeof(uint64_t))
    {
        //TODO 错误日志输出
        exit(-1);
    }
}


void Eventloop::wakeUp()
{
    uint64_t n = 1;
    int ret = write(wakeFd_, &n, sizeof(uint64_t));
    if (ret != sizeof(uint64_t))
    {
        //TODO 错误日志输出
        exit(-1);
    }
}


int Eventloop::getWakeupFd()
{
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd < 0)
    {
        //TODO 日志输出
        exit(-1);
    }
    return fd;
}

void Eventloop::loop()
{
    assert(quit_ == false);
    while (!quit_)
    {
        //poll返回值处理?
        int ret = poller_->poll(&activelist);
        printf("*debug* In Thread %d Poller Return\n", gettid());
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

/*
    11.10
    Eventloop的tid是只读对象,应该在初始化时就确定并且不再改变
    因此Eventloop需要是在线程上初始化的对象
    不能在主线程初始化
    
    下面这种传递tid方式会造成不安全的访问
    如果想要访问安全,之后每次访问tid_都必须加锁
    
*/

 /*
void Eventloop::runInThread()
{
    {
        MutexGuard mutex(mutex_);
        tid_ = base::gettid();
    }
    loop();
}
*/
/*
    11.10 new version
    将runInTHread移到ThreadLoop::runInThread(),使Eventloop在loop()中初始化
*/


void Eventloop::dotasks()
{
    taskQueue tmptasks;
    {
        MutexGuard mutex(mutex_);
        if (tasks.size() != 0)
            tmptasks.swap(tasks);
    }
    if (tmptasks.size() != 0)
    {
        printf("*debug* dotasks nums %d\n", tmptasks.size());
        for (auto task : tmptasks)
        {
            assert(task != nullptr);
            task();
        }
    }
}

void Eventloop::insertQueue(function<void()> func)
{
    {
        MutexGuard mutex(mutex_);
        tasks.push_back(std::move(func));
    }
    wakeUp();
}

void Eventloop::runInloop(function<void()> func)
{
    if (inloop())
    {
        printf("*debug* In Thread %d RunInloop Directly()\n", gettid());
        func();
    }
    else
    {
        printf("*debug* In Thread %d Cross Thread RunInloop()\n", gettid());
        insertQueue(std::move(func));//调用移动构造函数
    }
}

void Eventloop::update(Channel *channel)
{
    poller_->update(channel);
}