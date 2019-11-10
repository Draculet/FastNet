#ifndef __BASE_THREAD_H__
#define __BASE_THREAD_H__

#include <functional>
#include <pthread.h>
#include "Mutex.h"
#include <sys/types.h>

namespace base
{
class Thread;
struct trData
{
    trData(std::function<void()> func):
        runInThread(func)
    {

    }
    void run()
    {
        runInThread();
    }
    
    std::function<void()> runInThread;
    //TODO 是否保存
};
class Thread
{
    public:
    Thread(std::function<void()>);
    ~Thread();
    void start();
    static void *threadFunc(void *);

    private:
    trData td_;
    pthread_t pid_;
    //TODO 是否保存
    //Mutex mutex_;
    
};

}
#endif