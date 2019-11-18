#include "Thread.h"
#include <signal.h>

using namespace std;
using namespace base;

void *Thread::threadFunc(void *arg)
{
    //FIXME 放入全局
    ::signal(SIGPIPE, SIG_IGN);
    trData *td = static_cast<trData *>(arg);
    td->run();
}

Thread::~Thread()
{
    //TODO 判断是否join
    pthread_detach(pid_);
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

//TODO 加上join方法封装