#include "../Channel.h"
#include "../Eventloop.h"
#include "../Poller.h"
#include "../Thread.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
#include "NetAddr.h"
#include "Acceptor.h"
#include <assert.h>
#include <algorithm>
#include "Buffer.h"

using namespace std;
using namespace net;
using namespace base;

//Channel显然必须保存,各种调用信息都保存在Channel中,其他类都访问其指针
Mutex mutex_;
void eraseList(Connection *ch);
class Connection
{
    public:
    Connection(Channel chann):
        chann_(chann),
        buf_()
    {

    }

    void handleRead()
    {
        int ret = buf_.readFd(chann_.getFd());
        if (ret > 0)
        {
            userReadFunc_();
        }
        else if (ret == 0)
        {
            chann_->disableAll();
            close(chann_->getFd());
            //同步删除Channel的操作会使Channel::handleEvent()操作进行一半Channel就被析构,但是似乎并却不会出现core dump,将非常难以debug
            //eraseList(&chanList, ch);
            chann_->getLoop()->insertQueue(bind(::eraseList, this));//避免了Channeld的提前析构
        }
    }

    setUserReadFunc(function<void(Buffer *buf, Connection *)> func)
    {
        userReadFunc_ = func;
    }

    function<void(Buffer *buf, Connection *)> userReadFunc_;
    Buffer buf_;
    Channel chann_;
};

typedef vector<unique_ptr<Connection>> ConnList;
ConnList connList;
vector<Eventloop *> loopList;

int curLoop = 0;
const int kinitThread = 3;

void eraseList(Connection *conn)
{
    MutexGuard Mutex(mutex_);
    unique_ptr<Connection> tmp(conn);
    auto pos = find(connList.begin(), connList.end(), tmp);
    tmp.release();
    if (pos != connList.end())
    {
        printf("remove fd %d from thread %d\n", (*pos)->getFd(), gettid());
        connlist->erase(pos);
    }
}

void userReadFunc(Buffer *buf, Connection *conn)
{
    printf("Recv %d byte from peer\n", ret);
    string s = buf_.preViewAsString(buf_.readable());
    printf("Content:\n%s\n", s.c_str());
}

Eventloop *getNextLoop()
{
    int cur = curLoop;
    curLoop++;
    if (curLoop == kinitThread)
        curLoop = 0;
    return loopList[cur];
}

//仍旧有错,错误发生原因是channel被析构,猜测是同一个fd多次在epoll被激活,在epoll留有多个实例 //错误
//猜想是多线程造成错误
/* 
void userReadFunc(Channel *ch)
{
    assert(ch != nullptr);
    printf("Read In %d\n", gettid());
    char buf[1000];
    int ret = read(ch->getFd(), buf , 1000);
    printf("read from %d ret %d\n", ch->getFd(), ret);
    if (ret == 0)
    {
        ch->disableAll();
        close(ch->getFd());
        //同步删除Channel的操作会使Channel::handleEvent()操作进行一半Channel就被析构,但是似乎并却不会出现core dump,将非常难以debug
        //eraseList(&chanList, ch);

        ch->getLoop()->insertQueue(bind(eraseList, &chanList, ch));//避免了Channeld的提前析构
    }
}
*/

//也需要mutex保护
void acceptCallback(Channel *channel)
{
    sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    bzero(&clientaddr,sizeof(clientaddr));
    int connfd = accept(channel->getFd(), (struct sockaddr *) &clientaddr, &len);
    //此处有问题
    //Channel *newch = new Channel(connfd, getNextLoop());
    //用户自定义函数
    //TODO 临时实现Channel类的移动函数
    
    //移动有问题,并不会执行复制
    unique_ptr<Connection> newconnptr(new Connection(Channel(connfd, getNextLoop())));
    newchptr->chann_.setReadCallback(bind(userReadFunc, &*newchptr));
    newchptr->chann_.enableRead();
    MutexGuard mutex(mutex_);
    connList.push_back(move(newchptr));
}

//两个线程都关注stdin,互相抢缓冲区的数据
int main(void)
{
    Eventloop acceptloop;
    //Channel ch(STDIN_FILENO, &tloop);
    //ch.setReadCallback(bind(func, &ch));
    //ch.setWriteCallback(bind(writefunc, &ch));
    //ch.enableRead();

    //这种调用方法非常奇怪,需要封装到其他类中
    Eventloop loop1;
    Eventloop loop2;
    Eventloop loop3;

    loopList.push_back(&loop1);
    loopList.push_back(&loop2);
    loopList.push_back(&loop3);
    Thread th(bind(&Eventloop::runInThread, &acceptloop));
    Thread th1(bind(&Eventloop::runInThread, &loop1));
    Thread th2(bind(&Eventloop::runInThread, &loop2));
    Thread th3(bind(&Eventloop::runInThread, &loop3));
    th.start();
    th1.start();
    th2.start();
    th3.start();
    Acceptor acc(&acceptloop, 9981);
    //TODO 临时使用这种方法
    Channel *acceptch = acc.getChannel();
    acceptch->setReadCallback(bind(acceptCallback, acceptch));
    //                   //
    acc.listen();
    
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