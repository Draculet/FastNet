#ifndef __NET_SOCKET_H__
#define __NET_SOCKET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include "NetAddr.h"
#include "Noncopyable.h"

using namespace std;
using namespace net;

class Socket : base::noncopyable
{
    public:
    Socket(int fd, sockaddr_in addr):
        fd_(fd),
        addr_(addr)
    {

    }
    
    Socket(int port)
    {
        createNonBlockAndBind(port);
    }

    Socket(string ip, int port)
    {
        createNonBlockAndBind(ip, port);
    }

    ~Socket()
    {
        ::close(fd_);
    }

    void setNonBlock()
    {
        int flags = ::fcntl(fd_, F_GETFL, 0);
        flags |= O_NONBLOCK;
        int ret = ::fcntl(fd_, F_SETFL, flags);
        if (ret < 0)
        {
            perror("setNonBlock");
        }
        //TODO 判设置错误,日志输出
    }

    void setCloseExec()
    {
        flags = ::fcntl(fd_, F_GETFD, 0);
        flags |= FD_CLOEXEC;
        ret = ::fcntl(fd_, F_SETFD, flags);
        if (ret < 0)
        {
            perror("setCloseExec");
        }
        //TODO 判设置错误,日志输出
    }
    
    void createNonBlockAndBind(int port)
    {
        fd_ = ::socket(PF_INET, SOCK_STREAM, 0);
        setNonBlock();
        if (fd_ < 0)
        {
            perror("createNonBlockAndBind");
        }
        sockaddr_in servaddr;
        bzero(&servaddr,sizeof(&servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        int ret = ::bind(fd_, static_cast<sockaddr>servaddr, sizeof(servaddr));
        addr_ = servaddr;
        if (ret < 0)
        {
            perror("bind");
        }
        //TODO 判创建失败错,日志输出
    }

    void createNonBlockAndBind(string ip, int port)
    {
        fd_ = ::socket(PF_INET, SOCK_STREAM, 0);
        setNonBlock();
        if (fd_ < 0)
        {
            perror("createNonBlockAndBind");
        }
        sockaddr_in servaddr;
        bzero(&servaddr,sizeof(&servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        int ret = inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);
        if (ret < 9)
        {
            perror("inet_pton");
        }   
        int ret = ::bind(fd_, static_cast<sockaddr>servaddr, sizeof(servaddr));
        addr_ = servaddr;
        if (ret < 0)
        {
            perror("bind");
        }
        //TODO 判创建失败错,日志输出
    }

    int accept(NetAddr *addr)
    {
        sockaddr_in cli_addr;
        socklen_t len = sizeof(cli_addr);
        int connfd = ::accept4(fd_, &cli_addr, len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (connfd < 0)
        {
            //非阻塞accept,只输出错误信息
            perror("accept4");
            //TODO 判创建失败错,日志输出
            return -1;
        }
        addr->setAddr(cli_addr);
        return connfd;
    }

    void listen(int lsize = SOMAXCONN)
    {
        int ret = ::listen(fd_, lsize);
        if (ret < 0)
        {
            perror("listen");
        }
        //TODO 判监听失败,日志输出
    }

    void shutdown()
    {
        //shutdown默认只关闭写端,防止有信息未接受
        int ret = shutdown(fd_, SHUT_WR);
        if (ret < 0)
        {
            perror("shutdown");
        }
        //TODO 日志输出
    }

    void setNoDelay(bool on)
    {
        int optval = on ? 1 : 0;
        int ret = ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
        if (ret < 0)
        {
            perror("setNodelay");
        }
    }

    void setReuseAddr(bool on)
    {
        int optval = on ? 1 : 0;
        int ret = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        if (ret < 0)
        {
            perror("setReuseAddr");
        }
    }

    void setReusePort(bool on)
    {
        int optval = on ? 1 : 0;
        int ret = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
        if (ret < 0)
        {
            perror("reuseport");
        }
    }

    void setKeepAlive(bool on)
    {
        int optval = on ? 1 : 0;
        int ret = ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
        if (ret < 0)
        {
            perror("keepalive");
        }
    }

    private:
    int fd_;
    NetAddr addr_;
};

#endif