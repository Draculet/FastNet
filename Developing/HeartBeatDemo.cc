//#include "../Connection.h"
#include <iostream>
#include <stdio.h>
#include <list>
#include <memory>
#include <functional>
#include "../Eventloop.h"
#include "../Server.h"
#include "../Connection.h"
#include <set>

using namespace std;
using namespace net;
using namespace base;

class ConnNode
{
    public:
    ConnNode(shared_ptr<Connection> connptr)
        :connptr_(connptr)
    {
        
    }

    ~ConnNode()
    {
        shared_ptr<Connection> conn = connptr_.lock();
        if (conn)
        {
            //conn_->handleClose();
            cout << "~ConnNode()" << endl;
            cout << "Then Close The Connection" << endl;
        }
        else
        {
            cout << "The Connection Has Been Closed" << endl;
        }
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
        loop_runTimeEach(1, bind(&HeartBeat::updateList, this), "HeartBeat");
    }

    void updateList()
    {
        connList.push_front(set<shared_ptr<ConnNode>>());
        connList.pop_back();
    }

    void insertList(shared_ptr<Connection> conn)
    {
        shared_ptr<ConnNode> cn(new ConnNode(conn));
        connList.begin()->insert(cn);
    }

    private:
    list<set<shared_ptr<ConnNode>>> connList;
    Eventloop *loop_;
};

class HBServer
{
    public:
    HBServer()
        :serv_(new Server(4, 8832)),
        heart_(new HeartBeat(serv_.getLoop()))
    {

    }

    void onRead(Buffer *buf, shared_ptr<Connection> conn)
    {
        serv_->getLoop()->runInloop(bind(&HeartBeat::insertList, conn));
        string str = buf->preViewAsString(4);
        if (str == "PING")
        {
            conn->send("PONG");
        }
    }

    void start()
    {
        serv_->start();
        heart_.start();
    }


    private:
    unique_ptr<Server> serv_;
    unique_ptr<HeartBeat> heart_;
};