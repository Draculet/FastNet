#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <signal.h>
#include <map>
#include <set>
#include "../Server.h"
#include "../Connection.h"
#include "../Buffer.h"
#include "webSocketCodec.h"

using namespace std;
using namespace net;
using namespace base;

typedef enum{enable, disable} statType;

struct Pts
{
    Pts(int mfd, statType state = enable):
        mfd_(mfd),
        state_(state)
    {

    }

    int mfd_;
    string str_;
    statType state_;
};

class PtsPool
{
    public:
    PtsPool(int num = 1):
        pts_(),
        tmp_(-1, disable),
        num_(num),
        mutex_()
    {
        init();
    }

    ~PtsPool()
    {
        
    }

    //注意fork继承父进程打开的fd(包括listenfd)
    void init()
    {
        char buf[1024] = {0};
        vector<int> mfds;
        for (int i = 0; i < num_; i++)
        {
            int mfd = open("/dev/ptmx", O_RDWR);
            int flags = fcntl(mfd, F_GETFL, 0);
            flags |= O_NONBLOCK;  //设为非堵塞
            fcntl(mfd, F_SETFL, flags);
            printf("mfd : %d\n", mfd);
            grantpt(mfd);
            unlockpt(mfd);
            //查询并在控制台打印slave文件位置
            fprintf(stderr,"%s\n",ptsname(mfd));
            char **argv = (char**)malloc(sizeof(char*) * 2);
            argv[1] = ptsname(mfd);
            argv[0] = "/home/homulia/fastNet/remoteShellv2/loginExe";
            
            signal(SIGCHLD, SIG_IGN);//子进程可能死亡

            int pid = fork();//分为两个进程
            if(pid == 0)
            {
                //printf("Son Here\n");
                prctl(PR_SET_PDEATHSIG, SIGKILL);
                int res = execv("/home/homulia/fastNet/remoteShellv2/loginExe", argv);
                if (res < 0)
                {
                    printf("Pts %s StartUp Failde\n", argv[1]);
                    exit(-1);
                }
                else if (res >= 0)
                {
                    printf("Pts ret %d\n", res);
                    exit(-1);
                }
            }
            else
            {
                mfds.push_back(mfd);
            }
        }
        sleep(5); //FIXME 由于可能启动不成功,为了简单考虑暂时使用sleep尽量同步
        for (auto fd : mfds)
        {
            int ret = read(fd, buf, 1024);
            if (ret > 0)
            {
                printf("Fd %d Start Success\n", fd);
                pts_.push_back(Pts(fd));
            }
            else
                printf("Fd %d Exec Error\n", fd);
        }
        initStr_ = string(buf, strlen(buf));
    }

    Pts *getPts() //需要跨线程调用
    {
        int i;
        MutexGuard m(mutex_);
        for (i = 0; i < pts_.size(); i++)
        {
            if (pts_[i].state_ == enable)
            {
                pts_[i].state_ = disable;
                break;
            }
        }
        if (i == pts_.size())
        {
            //TODO 日志输出
            printf("*WARNNING*: Run out of Pts\n");
            return &tmp_;
        }
        else
        {
            return &pts_[i];
        }
    }

    void retBack(int mfd)
    {
        int i;
        MutexGuard m(mutex_);
        for (int i = 0; i < pts_.size(); i++)
        {
            if (pts_[i].mfd_ == mfd)
            {
                pts_[i].state_ = enable;
                break;
            }
        }
        if (i == pts_.size())
        {
            printf("*WARNNING*: retBack Failde\n");
        }
        return;
    }

    string initStr_;
    private:
    vector<Pts> pts_;
    Pts tmp_;
    int num_;
    Mutex mutex_;
};

class ShellServer
{
    public:
    ShellServer():
        ptspool_(),
        serv_(new Server(4, 8832)),
        ptsSet_(),
        ptsMap_(),
        connfdMap_(),
        codecMap_()
    {
        serv_->setReadCallback(bind(&ShellServer::onRead, this, placeholders::_1, placeholders::_2));
        serv_->setConnCallback(bind(&ShellServer::onConnction, this, placeholders::_1));
        serv_->setDisConnCallback(bind(&ShellServer::onDisConnection, this, placeholders::_1));
    }

    void onDisConnection(shared_ptr<Connection> conn)
    {
        /*
        *WARNNING* 如下注释掉的代码是偶然写出的错误代码,注意此处的connfdMap_[conn]虽然是临时对象,但是它复制了conn
        导致conn最后无法析构,使用map和shared_ptr协作注意这点

        if ( connfdMap_.find(connfdMap_[conn]) == connfdMap_.end() )
            return;
        */
        
        if ( connfdMap_.find(conn) == connfdMap_.end() )
            return;
        printf("*debug* onDisConnection\n");
        ptspool_.retBack( connfdMap_[conn]->getFd() );
        connfdMap_[conn]->getChannel()->disableAll();
        //chan_->disableAll();
        int fd = 3;
        printf("Connection Use_Count2 %d\n", ptsMap_[fd].use_count());

        MutexGuard m(mutex_);

        auto iter = connfdMap_.find(connfdMap_[conn]);
        if (iter != connfdMap_.end())
            connfdMap_.erase(iter);

        iter = connfdMap_.find(conn);
        if (iter != connfdMap_.end())
            connfdMap_.erase(iter);

        auto iter2 = codecMap_.find(conn);
        if (iter2 != codecMap_.end())
            codecMap_.erase(iter2);
        
        printf("Connection Use_Count %d\n", ptsMap_[fd].use_count());
        //此时conn析构,注意在conn的析构会导致mfd被socekt析构时close
    }

    void onConnction(shared_ptr<Connection> conn)//在connection被分配的线程上调用(connEstblish)
    {
        Eventloop *loop = serv_->getNextLoop();
        NetAddr addr;//zero

        //getPts已加锁
        Pts *pts = ptspool_.getPts();
        if (pts->mfd_ == -1)
        {
            printf("*debug* getPTS Failed\n");
            conn->handleClose();//同步调用
            return;
        }

        MutexGuard m(mutex_); // 上锁
        //考虑是否将其写成runInloop形式

        auto iter = ptsMap_.find(pts->mfd_);
        if (iter == ptsMap_.end())
        {
            shared_ptr<Connection> fdconn(new Connection(pts->mfd_, addr, loop));
            printf("PTS Fd %d Build New Connection\n", pts->mfd_);
            ptsMap_[pts->mfd_] = fdconn;
            connfdMap_[conn] = fdconn;
            connfdMap_[fdconn] = conn;
            ptsSet_.insert(fdconn);
            //conn->send(ptspool_.initStr_);

            //不设置ConnCallbcak
            fdconn->setReadCallback( bind(&ShellServer::onRead, this, placeholders::_1, placeholders::_2) );
            //fdconn->setWriteFinishCallBack(writeFinishCallBack_);
            fdconn->setCloseCallback( bind(&ShellServer::doRemoveConn, this) );
            //loop->runInloop( bind(&Connection::handleEstablish, fdconn) );
        }
        else
        {
            printf("PTS Fd %d Find Connection\n", pts->mfd_);
            shared_ptr<Connection> fdconn = ptsMap_[pts->mfd_];
            connfdMap_[conn] = fdconn;
            connfdMap_[fdconn] = conn;
            fdconn->getLoop()->runInloop( bind(&Channel::enableRead, fdconn->getChannel() ) );
        }

        
    }

    //TODO 双方的onRead整合在一起,可以选择将pts的onRead和conn的onRead的分开
    void onRead(Buffer *buf, shared_ptr<Connection> conn)
    {
        shared_ptr<webSocketCodec> codec;
        if (codecMap_.find(conn) == codecMap_.end())
        {
            codecMap_[conn] = shared_ptr<webSocketCodec>(new webSocketCodec(buf));
            codec = codecMap_[conn];
        }
        else
        {
            codec = codecMap_[conn];
        }
        if (ptsSet_.find(conn) == ptsSet_.end())
        {
            if (buf->preViewAsString(3) == "GET" || buf->preViewAsString(4) == "POST" ) //http
            {
                printf("*debug* 解析HTTP协议\n");
                requestParser reqhead(buf);//用法有误
                
                reqhead.parse();
                for (int i = 0; i < reqhead.size(); i++)
                {
                    if ( webSocketCodec::isWebSocket( &reqhead.reqs[i]) )
                        conn->send( webSocketCodec::toHttpResponse( &reqhead.reqs[i]) );
                }
                shared_ptr<Connection> ptsconn = connfdMap_[conn];
                ptsconn->getLoop()->runInloop( bind(&Connection::handleEstablish, ptsconn) );
                conn->send(frame::toFrame(ptspool_.initStr_, 1));
            }
            else //webSocket Frame
            {
                printf("*debug* 解析WebSocket Frame\n");
                int num = codec->parse();
                //string mark = "abcd";
                for (int i = 0; i < num; i++)
                {
                    codec->frames[i].debugPrint();
                    printf("\n");
                    if (codec->frames[i].getOpcode() == 1)
                    {
                        cout << "To Pts:\n" << codec->frames[i].getMessege() << endl;
                        if (codec->frames[i].getMessege().size() != 0)
                            connfdMap_[conn]->send(codec->frames[i].getMessege() + "\n");
                    }
                    else if (codec->frames[i].getOpcode() == 8)
                    {

                    }
                    else if (codec->frames[i].getOpcode() == 9)
                    {
                        //心跳帧
                        string data = "PONG";
                        printf("*debug* Pong");
                        conn->send(frame::toFrame(data, 10));
                    }
                    //codec->frames[i].debugPrint();
                    //conn->send(frame::toFrameWithMask(codec->frames[i].getMessege(), 1, mark));
                }
                codec->frames.clear();
            }
        }
        else //命令行返回字符
        {
            printf("*debug* 来自Pts的命令行字符\n");
            string res = buf->retrieveAllAsString();
            cout << "Result:\n" << res << endl;
            int flag = -1;
            for (int i = 0; i < res.size(); i++)
            {
                if (res[i] >= 0 && res[i] <= 31)
                    flag = 1;
            }
            connfdMap_[conn]->send(frame::toFrame(res, 1));
        }
    }

    void doRemoveConn()
    {
        printf("pts不会主动退出,需在来访连接断开时收尾!!\n");
    }

    void start()
    {
        serv_->start();
    }

    private:
    PtsPool ptspool_;
    unique_ptr<Server> serv_;
    set<shared_ptr<Connection> > ptsSet_;
    map<int, shared_ptr<Connection> > ptsMap_;//用于保存往pts连接
    map<shared_ptr<Connection>, shared_ptr<Connection> > connfdMap_;//要考虑连接关闭回收,需要server提供功能
    map<shared_ptr<Connection>, shared_ptr<webSocketCodec> > codecMap_;
    Mutex mutex_;
};

int main(void)
{
    ShellServer serv;
    sleep(1);
    serv.start();
    //PtsPool pts;
    //pts.init();
    /* 
    int mfd = pts.getPts();
    Thread th(std::bind(writeProcess, mfd));
    th.start();
    //pts.reset(mfd);
    while(1)
    {
        char buf[1] = {0};
        if(read(mfd,buf,1)>0)
            write(1,buf,1);
        else
            sleep(1);
    }
    */
}
