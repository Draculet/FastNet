#ifndef __THREADLOOPPOOL_H__
#define __THREADLOOPPOOL_H__

#include "Eventloop.h"
#include "ThreadLoop.h"

using namespace std;
using namespace net;

class ThreadLoopPool
{
    public:
    ThreadLoopPool(size_t threadnums = 4):
        threadnums_(threadnums),
        next_(0),
        state_(kInit)
    {
        
    }

    ~ThreadLoopPool()
    {
        state_ = kQuit;
    }

    void start()
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

    Eventloop *getNextLoop()
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

    vector<Eventloop *> *getLoops()
    {
        return &loops_;
    }

    private:
    vector<unique_ptr<ThreadLoop> > threads_;
    vector<Eventloop *> loops_;
    size_t threadnums_;
    size_t next_;
    enum {kRun, kQuit, kInit} state_;
};

#endif