#include "../include/Acceptor.h"

using namespace net;
using namespace std;

    Acceptor::Acceptor(Eventloop *loop, int port)
        :acceptloop_(loop),
        acceptsoc_(new Socket(port)),   //socket默认setReuse,setNonBlock
        acceptchan_(new Channel(acceptsoc_->getFd(), loop)),
        extraFd_(open("/dev/null", O_RDWR))
    {}

    Acceptor::~Acceptor()
    {
        acceptchan_->disableAll();
        ::close(extraFd_);
    }

    int Acceptor::getAcceptFd()
    {
        return acceptsoc_->getFd();
    }

    void Acceptor::setNewConnCallback(function<void (int, NetAddr&)> newConnCallback)
    {
        newConnCallback_ = newConnCallback;
    }
    
    void Acceptor::listen()
    {
        acceptchan_->setReadCallback(bind(&Acceptor::handleRead, this));
        acceptsoc_->listen();
        acceptchan_->enableRead();
    }

    void Acceptor::handleRead()
    {
        NetAddr addr;
        int sockfd = acceptsoc_->accept(&addr);
        if (sockfd < 0)//返回-1,可能是异步正常返回,也可能是fd用完
        {
            if (errno == EMFILE)
            {
                //TODO 日志输出
                NetAddr tmp;
                ::close(extraFd_);
                extraFd_ = acceptsoc_->accept(&tmp);
                ::close(extraFd_);
                extraFd_ = ::open("/dev/null", O_RDWR);
            }
        }
        else//当标准输入关闭时,sockfd可能返回0
        {
            if (newConnCallback_)
                newConnCallback_(sockfd, addr);
            else//出现错误
            {
                ::close(sockfd);
            }
        }
    }