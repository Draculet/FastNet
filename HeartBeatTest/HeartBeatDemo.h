//#include "../Connection.h"
#include <iostream>
#include <stdio.h>
#include <list>
#include <memory>
#include <functional>
#include <set>
#include "../Eventloop.h"
#include "../Server.h"
#include "../Connection.h"

using namespace std;
using namespace net;
using namespace base;

class ConnNode
{
    public:
    ConnNode(shared_ptr<Connection> connptr)
        :connptr_(connptr)
    {
        cout << "ConnNode()" << endl;
    }

    ~ConnNode()
    {
        shared_ptr<Connection> conn = connptr_.lock();
        if (conn)
        {
            //conn_->handleClose();
            conn->getLoop()->runInloop(bind(&Connection::handleClose, conn));
            cout << "~ConnNode()" << endl;
            cout << "__________________Then Close The Connection_______________________" << endl;
        }
        else
        {
            cout << "The Connection Has Been Closed" << endl;
        }
    }

    shared_ptr<Connection> getPointer()
    {
        return connptr_.lock();
    }

    private:
    weak_ptr<Connection> connptr_;
};

class HeartBeat
{
    public:
    HeartBeat(Eventloop *loop)
        :connList(10), //10s
        loop_(loop)
    {
        
    }

    void start()
    {
        loop_->runTimeEach(1, bind(&HeartBeat::cutLife, this), "HeartBeat");
    }

    void addHeartBeat(shared_ptr<Connection> conn)
    {
        loop_->runInloop(bind(&HeartBeat::createNodeInThread, this, conn));
    }

    void removeHeartBeat(shared_ptr<Connection> conn)
    {
        loop_->runInloop(bind(&HeartBeat::removeNodeInThread, this, conn));
    }

    void extendLife(shared_ptr<Connection> conn)
    {
        loop_->runInloop(bind(&HeartBeat::insertListInThread, this, conn));
    }

    void cutLife()
    {
        cout << "updateList()" << endl;
        connList.push_front(set<shared_ptr<ConnNode>>());
        connList.pop_back();
    }


    //OnConnect 建立连接和ConnNode对应关系
    void createNodeInThread(shared_ptr<Connection> conn)
    {
        auto iter = connMap_.find(conn);
        if (iter == connMap_.end())
        {
            shared_ptr<ConnNode> nsptr(new ConnNode(conn));
            weak_ptr<ConnNode> nwptr = nsptr;
            connMap_[conn] = nwptr;
            connList.begin()->insert(nsptr);
        }
        else
        {
            //TODO 出错,日志
        }
    }

    //OnDisConnect 让shared_ptr<Connection>能正常析构
    void removeNodeInThread(shared_ptr<Connection> conn)
    {
        auto iter = connMap_.find(conn);
        if (iter != connMap_.end())
        {
            connMap_.erase(iter);
        }
        else
        {
            //TODO 出错,日志
        }
    }


    //OnRead
    void insertListInThread(shared_ptr<Connection> conn)
    {
        //weak_ptr<Connection> wptr = conn;
        auto iter = connMap_.find(conn);
        if (iter != connMap_.end())
        {
            shared_ptr<ConnNode> nsptr = connMap_[conn].lock();
            if (nsptr)
            {
                printf("*debug* Insert Conn Node\n");
                connList.begin()->insert(nsptr);
            }
            //ConnNode已析构,无需处理
        }
        else
        {
            //TODO 出错,日志记录
        }
    }
    

    /* 
    void insertList(shared_ptr<Connection> conn)
    {
        shared_ptr<ConnNode> cn(new ConnNode(conn));
        connList.begin()->insert(cn);
    }
    */

    private:
    list<set<shared_ptr<ConnNode>>> connList;
    //map<weak_ptr<Connection>, weak_ptr<ConnNode>> connMap_;
    map<shared_ptr<Connection>, weak_ptr<ConnNode>> connMap_;
    Eventloop *loop_;
};

/* 
bool operator<(weak_ptr<Connection> cnptr1, weak_ptr<Connection> cnptr2)
{
    shared_ptr<Connection> conn1 = cnptr1.lock();
    shared_ptr<Connection> conn2 = cnptr2.lock();
    if (conn1 && conn2)
        return conn1 < conn2;
    else
        return false;
}
*/

class HBServer
{
    public:
    HBServer()
        :serv_(new Server(4, 8832)),
        heart_(new HeartBeat(serv_->getLoop()))
    {
        serv_->setReadCallback(bind(&HBServer::onRead, this, placeholders::_1, placeholders::_2));
        serv_->setConnCallback(bind(&HBServer::onConnect, this, placeholders::_1));
        serv_->setDisConnCallback(bind(&HBServer::onDisConnect, this, placeholders::_1));
    }

    void onConnect(shared_ptr<Connection> conn)
    {
        cout << "onConnect()" << endl;
        heart_->addHeartBeat(conn);
    }

    void onDisConnect(shared_ptr<Connection> conn)
    {
        cout << "onDisConnect()" << endl;
        heart_->removeHeartBeat(conn);
    }

    void onRead(Buffer *buf, shared_ptr<Connection> conn)
    {
        heart_->extendLife(conn);
        string str = buf->preViewAsString(4);
        if (str == "PING")
        {
            buf->retrieve(4);
            conn->send("PONG");
        }
    }

    void start()
    {
        heart_->start();
        serv_->start();
    }


    private:
    unique_ptr<Server> serv_;
    unique_ptr<HeartBeat> heart_;
};