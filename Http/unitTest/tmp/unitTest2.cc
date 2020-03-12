#include "../Channel.h"
#include "../Eventloop.h"
#include "../Poller.h"
#include "../Thread.h"
#include <stdio.h>

using namespace std;
using namespace net;
using namespace base;

void func(Channel *ch)
{
    printf("Read\n");
    char buf[1000];
    int ret = read(ch->getFd(), buf , 1);
    printf("read ret %d\n", ret);
}
void func1(Channel *ch)
{
    printf("Read1\n");
    char buf[1000];
    int ret = read(ch->getFd(), buf , 1);
    printf("read1 ret %d\n", ret);
}
void writefunc(Channel *ch)
{
    printf("write\n");
    ch->disableWrite();
}

//两个线程都关注stdin,互相抢缓冲区的数据
int main(void)
{
    Eventloop loop;
    Eventloop loop1;
    Channel ch(STDIN_FILENO, &loop);
    Channel ch1(STDIN_FILENO, &loop1);
    ch.setReadCallback(bind(func, &ch));
    ch1.setReadCallback(bind(func1, &ch1));
    ch.setWriteCallback(bind(writefunc, &ch));
    ch1.setWriteCallback(bind(writefunc, &ch1));
    //ch.disableRead();
    ch.enableRead();
    ch1.enableRead();
    //ch1.enableRead();
    Thread th(bind(&Eventloop::runInThread, &loop));
    Thread th1(bind(&Eventloop::runInThread, &loop1));
    th.start();
    th1.start();
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