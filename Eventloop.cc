#include <stdio.h>
#include "Eventloop.h"
#include "Gettid.h"
#include <assert.h>
#include "Channel.h"

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
    wakechan_->enableRead();
}

Eventloop::~Eventloop()
{
    wakechan_->disableAll();
    ::close(wakeFd_);
}

/*
    FIXME 错误使用!!

void Eventloop::handleRead()
{
    char buf[128] = {0};
    int ret = read(wakeFd_, buf, 128);//保证将缓冲区清空
    if (ret != sizeof(int))
    {
        //错误日志输出
    }
}

/*
    FIXME 错误使用!!
void Eventloop::wakeUp()
{
    int num = 8;
    int ret = write(wakeFd_, &num, sizeof(int));
    if (ret != s.size())
    {
        //错误日志输出
    }
}
*/

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
    将runInTHread移到ThreadLoop
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
        for (auto task : tmptasks)
        {
            printf("debug dotask\n");
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
        func();
    else
        insertQueue(std::move(func));//调用移动构造函数
}

void Eventloop::update(Channel *channel)
{
    poller_->update(channel);
}