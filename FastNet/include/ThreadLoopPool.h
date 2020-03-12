#ifndef __THREADLOOPPOOL_H__
#define __THREADLOOPPOOL_H__

#include "Eventloop.h"
#include "ThreadLoop.h"

namespace net
{
class ThreadLoopPool
{
    public:
    ThreadLoopPool(size_t threadnums = 4);
    ~ThreadLoopPool();
    void start();
     Eventloop *getNextLoop();
    std::vector< Eventloop *> *getLoops();
    
    private:
    std::vector<std::unique_ptr< ThreadLoop> > threads_;
    std::vector< Eventloop *> loops_;
    size_t threadnums_;
    size_t next_;
    enum {kRun, kQuit, kInit} state_;
};
}
#endif