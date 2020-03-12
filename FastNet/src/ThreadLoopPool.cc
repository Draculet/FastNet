#include "../include/ThreadLoopPool.h"
using namespace std;
using namespace net;
using namespace base;

ThreadLoopPool::ThreadLoopPool(size_t threadnums):
    threadnums_(threadnums),
    next_(0),
    state_(kInit)
{
    
}

ThreadLoopPool::~ThreadLoopPool()
{
    state_ = kQuit;
}

void ThreadLoopPool::start()
{
    state_ = kRun;
    for (int i = 0; i < threadnums_; i++)
    {
        threads_.push_back(unique_ptr<ThreadLoop>(new ThreadLoop() ) );
    }
    for (auto &thread : threads_)
    {
        thread->start();
        Eventloop *loop = nullptr;
        loop = thread->getLoop();
        if (loop)
            loops_.push_back(loop);
        else
        {
            //TODO 日志输出报错
        }
    }
}

Eventloop *ThreadLoopPool::getNextLoop()
{
    Eventloop *loop;
    if (!loops_.empty())
    {
        loop = loops_[next_];
        next_++;
        if (next_ >= loops_.size())
            next_ = 0;
    }
    else
    {
        //日志输出报错
        threadnums_ = 4;
        next_ = 0;
        start();
        loop = loops_[next_];
        next_++;
        if (next_ >= loops_.size())
            next_ = 0;
    }

    return loop;
}

vector<Eventloop *> *ThreadLoopPool::getLoops()
{
    return &loops_;
}