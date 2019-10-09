#ifndef __NET_NETADDR_H__
#define __NET_NETADDR_H__
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

namespace net
{
    class NetAddr
    {
        public:
        NetAddr(int port);
        NetAddr(int ,sockaddr_in);
        sockaddr_in getAddr(){return addr_;}
        int getPort(){return addr_.sin_port;};
        int getFd(){return fd_;};
        private:
        sockaddr_in addr_;
        int fd_;
    };
}

#endif