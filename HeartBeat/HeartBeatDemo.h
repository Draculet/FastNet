#include <iostream>
#include <stdio.h>
#include <list>
#include <memory>
#include <functional>
#include <set>
#include "../FastNet/include/Eventloop.h"
#include "../FastNet/include/Server.h"
#include "../FastNet/include/Connection.h"


namespace net
{
class ConnNode
{
    public:
    ConnNode(std::shared_ptr<Connection> connptr);
    ~ConnNode();
    std::shared_ptr<Connection> getPointer();

    private:
    std::weak_ptr<Connection> connptr_;
};

class HeartBeat
{
    public:
    HeartBeat(Eventloop *loop);
    void start();
    void addHeartBeat(std::shared_ptr<Connection> conn);
    void removeHeartBeat(std::shared_ptr<Connection> conn);
    void extendLife(std::shared_ptr<Connection> conn);
    void cutLife();
    //OnConnect 建立连接和ConnNode对应关系
    void createNodeInThread(std::shared_ptr<Connection> conn);
    //OnDisConnect 让shared_ptr<Connection>能正常析构
    void removeNodeInThread(std::shared_ptr<Connection> conn);
    //OnRead
    void insertListInThread(std::shared_ptr<Connection> conn);
    /* 
    void insertList(shared_ptr<Connection> conn)
    {
        shared_ptr<ConnNode> cn(new ConnNode(conn));
        connList.begin()->insert(cn);
    }
    */

    private:
    std::list<std::set<std::shared_ptr<ConnNode>>> connList;
    //map<weak_ptr<Connection>, weak_ptr<ConnNode>> connMap_;
    std::map<std::shared_ptr<Connection>, std::weak_ptr<ConnNode>> connMap_;
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
    HBServer();
    void onConnect(std::shared_ptr<Connection> conn);
    void onDisConnect(std::shared_ptr<Connection> conn);
    void onRead(Buffer *buf, std::shared_ptr<Connection> conn);
    void start();

    private:
    std::unique_ptr<Server> serv_;
    std::unique_ptr<HeartBeat> heart_;
};

}