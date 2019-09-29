#include "Thread.h"

using namespace std;
using namespace base;

void *Thread::threadFunc(void *arg)
{
    trData *td = static_cast<trData *>(arg);
    td->run();
}

Thread::Thread(function<void()> func):
    td_(func)
    {

    }

void Thread::start()
{
    int ret = pthread_create(&pid_, NULL, Thread::threadFunc, static_cast<void *>(&td_));
    //TODO 处理ret
}