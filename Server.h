#include "Acceptor.h"
#include "Eventloop.h"
#include "ThreadLoop.h"
#include "ThreadLoopPool.h"

using namespace std;

class Server : base::noncopyable
{
    public:
    //使用主线程运行Server, Eventloop属于主线程
    Server(int threadnums, int port):
        loop_(new Eventloop()),
        pool_(threadnums),
        acceptor_(&*loop_, port)
    {
        
    }

    ~Server()
    {
        
    }

    //不能跨线程调用,同步执行
    void start()
    {
        acceptor_->listen();
        //不用担心丢失连接请求,测试见~/netDesign/epollTest/epollTest2.cc
        pool_->start();
        loop_->loop();
    }

    void handleNewConn(int sockfd, NetAddr &addr)
    {
        Eventloop *loop = pool_->getNextLoop();
        shared_ptr<Connection> conn(new Connection(sockfd, addr, loop));
        connlist.push_back(conn);
        conn->setReadCallback(readCallback);
        conn->setWriteCompleteCallback(writeCompleteCallback);//conn中未实现
        conn->setCloseCallback(bind(this, removeConn, _1));
        loop->runInloop(bind(&Connection::handleEstablish, conn));
    }

    void doRemoveConn(shared_ptr<Connection> conn)//由其他线程调用
    {
        loop_->runInloop(bind(&Connection::removeConn, this, conn));
    }

    void removeConn(shared_ptr<Connection> conn)
    {
        //需要map来方便删除
    }

    private:
    unique_ptr<Eventloop> loop_;
    unique_ptr<ThreadLoopPool> pool_;
    unique_ptr<Acceptor> acceptor_;
    vector<shared_ptr<Connection> > connlist;//TODO 改成map<>
    
}