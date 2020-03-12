#ifndef __THREAD_LOOP_H__
#define __THREAD_LOOP_H__

#include "Thread.h"
#include "Eventloop.h"
#include "Mutex.h"
#include "Condition.h"
#include <memory>
#include <functional>


namespace net
{
class ThreadLoop
{
    public:
    ThreadLoop();
    ~ThreadLoop();
     Eventloop *getLoop();
    void start();
    void runInThread();

    private:
     Eventloop *loop_;
    std::unique_ptr<base::Thread> thread_;
    base::Mutex mutex_;
    base::Condition cond_;
    enum {kRun, kQuit} state_;
};
}

#endif