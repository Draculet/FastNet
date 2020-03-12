#include "RelayServer.h"

using namespace std;
using namespace net;
using namespace base;

extern string ip;
extern int port;

int connectPeer(string ip, int port, NetAddr *addr)
{
    struct sockaddr_in serveraddr;
    int clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientFd < 0)
    {
        perror("socket");
    }
    bzero(&serveraddr,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serveraddr.sin_addr);
    int ret = connect(clientFd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if(ret != 0)
    {
    	close(clientFd);
    	perror("connect");
        return -1;
	}

    *addr = NetAddr(serveraddr);
    return clientFd;
}

/* TODO 通过JSON配置转发的远程主机信息 */
/* 远程主机的ip,port */


RelayServer::RelayServer(int port):
    serv_(new Server(4, port)),
    connfdMap_(),
    mutex_()
{
    serv_->setReadCallback(bind(&RelayServer::onRead, this, placeholders::_1, placeholders::_2));
    serv_->setConnCallback(bind(&RelayServer::onConnection, this, placeholders::_1));
    serv_->setDisConnCallback(bind(&RelayServer::onDisConnection, this, placeholders::_1));
}

void RelayServer::onRead(Buffer *buf, shared_ptr<Connection> conn)
{
    string data = buf->retrieveAllAsString();
    connfdMap_[conn]->send(data);
}

void RelayServer::onDisConnection(shared_ptr<Connection> conn)
{
    //此段程序处理当连接建立后去connect client时失败直接调用handleClose的情况
    if ( connfdMap_.find(conn) == connfdMap_.end() )
        return;

    printf("*debug* onDisConnection\n");
    //connfdMap_[conn]->getChannel()->disableAll();
    //chan_->disableAll();

    //shared_ptr保证在connection析构之前disableAll成功,从epoll中删除connfd
    connfdMap_[conn]->getLoop()->runInloop( bind(&RelayServer::disableConn, this, connfdMap_[conn]) );
    MutexGuard m(mutex_);

    auto iter = connfdMap_.find(connfdMap_[conn]);
    if (iter != connfdMap_.end())
        connfdMap_.erase(iter);
    iter = connfdMap_.find(conn);
    if (iter != connfdMap_.end())
        connfdMap_.erase(iter);
}

//client loop runInloop
void RelayServer::disableConn(shared_ptr<Connection> conn)
{
    //用于测试析构是否先于disable sleep(5);
    conn->getChannel()->disableAll();
}

void RelayServer::onConnection(shared_ptr<Connection> conn)
{
    Eventloop *loop = serv_->getNextLoop();
    NetAddr addr;
    //ssh -R localhost:8833:localhost:80 root@66.42.68.142 以8833作为远程端口转发的端口
    //client建立与localhost:8833的连接,同时填上addr
    int sockfd = connectPeer(ip, port, &addr);   
    printf("New Connection Fd %d\n", sockfd);
    if (sockfd < 0)
    {
        conn->handleClose();
        return;
    }
    MutexGuard m(mutex_); // 上锁
    //考虑是否将其写成runInloop形式

    shared_ptr<Connection> fdconn(new Connection(sockfd, addr, loop));
    connfdMap_[conn] = fdconn; //connfdMap_保证了conn的存活
    connfdMap_[fdconn] = conn;
    //此处使用相同的onRead,其可读回调逻辑相同
    fdconn->setReadCallback( bind(&RelayServer::onRead, this, placeholders::_1, placeholders::_2) );
        //fdconn->setWriteFinishCallBack(writeFinishCallBack_);
    fdconn->setCloseCallback( bind(&RelayServer::clientClose, this, placeholders::_1) );
    loop->runInloop( bind(&Connection::handleEstablish, fdconn) );
}

void RelayServer::clientClose(shared_ptr<Connection> conn)//由其他线程调用
{
    //new verson
    connfdMap_[conn]->getLoop()->runInloop(bind( &Connection::handleClose, connfdMap_[conn]) );
}

void RelayServer::start()
{
    serv_->start();
}

    //
    //
    //前部分程序对于connfdMap_已经使用锁同步,此处也需用锁
    //FIXME 可将前面修改connfdMap_程序及此处改成runInloop
    //MutexGuard m(mutex_);
    //auto iter = connfdMap_.find(conn);
    //if (iter != connfdMap_.end())
    //{
    //    connfdMap_.erase(iter);
    //}
    //else
    //{
    //    //TODO 报错
    //    printf("*debug* Error In Remove Connection\n");
    //}
    //client connect的对端会主动关闭连接
    //client Connect的对端主动close时调用该函数
    //TODO 从connfdMap_删去conn,同时断开主动连接方的连接
    //...
    //serv_->getLoop()->runInloop(bind(RelayServer::removeConn, this, conn));
    //
    //


/* 
void removeConn(shared_ptr<Connection> conn)
{
    int ret = connfdMap_.erase(conn->getPeerAddr().toString());
    if (ret != 1)
    {
        //TODO 报错
        printf("*debug* Error In Remove Connection\n");
    }
    //正常情况下接下来将析构Connection了
}
*/

