#include "../Eventloop.h"
#include "../ThreadLoop.h"
#include <iostream>
using namespace std;
using namespace net;
using namespace base;
Eventloop *loop[4];
void run(int sec, int i);

void callback(int sec, Eventloop *loop, int i)
{
    printf("__________________Pid %d TimeOut_______________________\n", gettid());
    run(sec, i);
}

void run(int sec, int i)
{
    Eventloop *l = loop[i];
    i = ++i % 4;
    l->runTimeAfter(sec, bind(callback, sec + 1, loop[i], i), "hello");
}

void call()
{
    printf("__________________Pid %d TimeOut_______________________\n", gettid());
}

void conCallback(int sec)
{
    loop[0]->runTimeAfter(sec, call, "hello");
}

int main(void)
{
    Eventloop lo;
    ThreadLoop th[4];
    th[0].start();
    th[1].start();
    th[2].start();
    th[3].start();
    loop[0] = th[0].getLoop();
    loop[1] = th[1].getLoop();
    loop[2] = th[2].getLoop();
    loop[3] = th[3].getLoop();
    loop[0]->runTimeAfter(10, call, "hello");
    while (1)
    {
        loop[1]->runInloop(bind(conCallback, 5));
        loop[2]->runInloop(bind(conCallback, 10));
        loop[3]->runInloop(bind(conCallback, 15));
        lo.runInloop(bind(conCallback, 1));
        sleep(20);
        printf("wakeup! \n");
    }
    

    lo.loop();
}