#include "../Channel.h"
#include "../Eventloop.h"
#include "../Poller.h"
#include "../Thread.h"
#include <stdio.h>
#include <sys/socket.h>

using namespace std;
using namespace net;
using namespace base;



//两个线程都关注stdin,互相抢缓冲区的数据
int main(void)
{
    Eventloop tloop;
    //Channel ch(STDIN_FILENO, &tloop);
    //ch.setReadCallback(bind(func, &ch));
    //ch.setWriteCallback(bind(writefunc, &ch));
    //ch.enableRead();
    Thread th(bind(&Eventloop::runInThread, &tloop));
    th.start();
    
    while(1)
    {
        /*
        ch.disableRead();
        sleep(5);
        ch.disableWrite();
        sleep(5);
        ch.enableRead();
        sleep(5);
        ch.enableWrite();
        sleep(5);
        ch.disableWrite();
        sleep(5);
        ch.disableAll();
        sleep(5);
        */
    }
}