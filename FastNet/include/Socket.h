#ifndef __NET_SOCKET_H__
#define __NET_SOCKET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include "NetAddr.h"
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "Noncopyable.h"

namespace net
{
class Socket : base::noncopyable
{
    public:
    Socket(int fd,  NetAddr addr);
    Socket(Socket &&tmp);
    Socket(int port);
    Socket(std::string ip, int port);
    ~Socket();
    void setNonBlock();
    void setCloseExec();
    void createNonBlockAndBind(int port);
    void createNonBlockAndBind(std::string ip, int port);
    int accept( NetAddr *addr);
    void listen(int lsize = SOMAXCONN);
    void shutdownWR();
    void setNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
     NetAddr *getAddr();
    int getFd();

    private:
    int fd_;
     NetAddr addr_;
};
}

#endif