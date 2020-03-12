#include "../include/Server.h"
using namespace std;
using namespace base;
using namespace net;


    //使用主线程运行Server, Eventloop属于主线程
    Server::Server(int threadnums, int port):
        loop_(new Eventloop()),
        pool_(new ThreadLoopPool(threadnums)),
        acceptor_(new Acceptor(&*loop_, port))
    {
        acceptor_->setNewConnCallback(bind(&Server::handleNewConn, this, placeholders::_1, placeholders::_2));
    }

    Server::~Server()
    {
        
    }

    int Server::getAcceptFd()
    {
        return acceptor_->getAcceptFd();
    }

    //不能跨线程调用,同步执行
    void Server::start()
    {
        printf("*debug* Server Start\n");
        acceptor_->listen();
        //不用担心丢失连接请求,测试见~/netDesign/epollTest/epollTest2.cc
        pool_->start();
        loop_->loop();
    }

    void Server::handleNewConn(int sockfd, NetAddr &addr)
    {
        //testPrint();
        printf("*debug* handleNewConn\n");
        printf("New Connection Fd %d\n", sockfd);
        Eventloop *loop = pool_->getNextLoop();
        if (loop)
            printf("*debug* getloop\n");
        shared_ptr<Connection> conn(new Connection(sockfd, addr, loop));
        //connlist.push_back(conn);
        connMap_[addr.toString()] = conn;
        conn->setConnCallback(connCallback_);
        conn->setDisConnCallback(disConnCallback_);
        conn->setReadCallback(readCallback_);
        if (writeFinishCallBack_)
        {
            //默认writeFinishCallback
        }
        conn->setWriteFinishCallBack(writeFinishCallBack_);
        conn->setCloseCallback(bind(&Server::doRemoveConn, this, placeholders::_1));
        loop->runInloop(bind(&Connection::handleEstablish, conn));
    }

    void Server::doRemoveConn(shared_ptr<Connection> conn)//由其他线程调用
    {
        loop_->runInloop(bind(&Server::removeConn, this, conn));
    }

    void Server::removeConn(shared_ptr<Connection> conn)
    {
        int ret = connMap_.erase(conn->getPeerAddr().toString());
        if (ret != 1)
        {
            //TODO 报错
            printf("*debug* Error In Remove Connection\n");
        }
        //正常情况下接下来将析构Connection了
    }
    
    Eventloop *Server::getLoop()
    {
        return &*loop_;
    }
    
    void Server::setConnCallback(function<void (shared_ptr<Connection>)> connCallback)
    {
        connCallback_ = connCallback;
    }

    void Server::setDisConnCallback(function<void (shared_ptr<Connection>)> disConnCallback)
    {
        disConnCallback_ = disConnCallback;
    }

    void Server::setReadCallback(function<void (Buffer *, shared_ptr<Connection>) > readCallback)
    {
        readCallback_ = readCallback;
    }

    void Server::setWriteFinishCallBack(function<void ()> writeFinishCallBack)
    {
        writeFinishCallBack_ = writeFinishCallBack;        
    }

    map<string, shared_ptr<Connection> > *Server::getConnMap()
    {
        return &connMap_;
    }

    void Server::testPrint()
    {
        for (auto iter = connMap_.begin(); iter != connMap_.end(); iter++)
        {
            cout << "Connection:" << iter->first << " input:" << iter->second->getInputSize() << " output:" << iter->second
            ->getOutputSize() << endl;
        }
    }

    Eventloop *Server::getNextLoop()
    {
        return pool_->getNextLoop();
    }