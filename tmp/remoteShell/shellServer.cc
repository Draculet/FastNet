#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "../Server.h"
#include "../Connection.h"
#include "../Buffer.h"
#include <map>

using namespace std;
using namespace net;
using namespace base;

class ShellServer
{
    public:
    ShellServer():
        serv_(new Server(2, 8832)),
        ffd_(),
        connfdMap_()
    {
        serv_->setReadCallback(bind(&ShellServer::onRead, this, placeholders::_1, placeholders::_2));
        serv_->setConnCallback(bind(&ShellServer::onConnction, this, placeholders::_1));
    }

    void onConnction(shared_ptr<Connection> conn)
    {
        printf("=====OnConnection====\n");
        int mfd = open("/dev/ptmx", O_RDWR);
        grantpt(mfd);
        unlockpt(mfd);
        //查询并在控制台打印slave文件位置
        fprintf(stderr,"%s\n",ptsname(mfd));
        char **argv = (char**)malloc(sizeof(char*) * 2);
        argv[1] = ptsname(mfd);
        argv[0] = "/home/homulia/fastNet/remoteShell/loginExe";
        int pid = fork();//分为两个进程
        if(pid == 0)
        {
            close(serv_->getAcceptFd());
            //printf("Son Here\n");
            execv("/home/homulia/fastNet/remoteShell/loginExe", argv);
            while(1)
            {}
        }
        
        //int mfd = 0;
        Eventloop *loop = serv_->getNextLoop();
        NetAddr addr;//zero
        shared_ptr<Connection> fdconn(new Connection(mfd, addr, loop));
        ffd_.push_back(fdconn);
        connfdMap_[conn] = fdconn;
        connfdMap_[fdconn] = conn;
        //不设置ConnCallbcak
        fdconn->setReadCallback( bind(&ShellServer::onRead, this, placeholders::_1, placeholders::_2) );
        //fdconn->setWriteFinishCallBack(writeFinishCallBack_);
        fdconn->setCloseCallback( bind(&ShellServer::doRemoveConn, this) );
        loop->runInloop( bind(&Connection::handleEstablish, fdconn) );
        //loop->runInloop( bind(&ShellServer::onConnction, this, conn) );
    }

    void onRead(Buffer *buf, shared_ptr<Connection> conn)
    {
        string data = buf->retrieveAllAsString();
        //cout << "=========data: " << data << endl;
        //string *s = new string(data);
        connfdMap_[conn]->send(data);
    }

    void doRemoveConn()
    {
        printf("Remove\n");
    }

    void start()
    {
        serv_->start();
    }
    private:
    unique_ptr<Server> serv_;
    vector<shared_ptr<Connection> > ffd_;
    map<shared_ptr<Connection>, shared_ptr<Connection> > connfdMap_;//要考虑连接关闭回收,需要server提供功能
};

int main(int argc)
{
    ShellServer serv;
    serv.start();
}
