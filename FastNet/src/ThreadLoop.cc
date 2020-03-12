#include "../include/ThreadLoop.h"
using namespace std;
using namespace net;
using namespace base;

ThreadLoop::ThreadLoop():
    loop_(nullptr),
    thread_(new Thread(bind(&ThreadLoop::runInThread, this))),
    mutex_(),
    cond_(mutex_),
    state_(kRun)
{

}

ThreadLoop::~ThreadLoop()
{
    //TODO 填充细节处理,日志输出
    //线程的析构交由Thread处理
}

Eventloop *ThreadLoop::getLoop()
{
    return loop_; 
}

void ThreadLoop::start()
{
    state_ = kRun;
    thread_->start();
    MutexGuard mutex(mutex_);
    while (loop_ == nullptr)
    {
        cond_.wait();
    }
}

/* 
    11.10
    将EventLoop中的runInThread移到此处,方便直接用锁
*/
void ThreadLoop::runInThread()
{
    Eventloop loop;
    /*
        FIXME

        此处初始化Eventloop,tid_初始化时确定,并且不会改变
        后面跨线程访问是否会有问题,是否应该加锁?
        */
    {
        MutexGuard mutex(mutex_);
        loop_ = &loop;
        cond_.notify();
    }
    loop.loop();
    MutexGuard mutex(mutex_);
    loop_ = nullptr;
    state_ = kQuit;
}