#ifndef __NET_NETADDR_H__
#define __NET_NETADDR_H__
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fcntl.h>


namespace net
{
    class NetAddr
    {
        public:
        NetAddr(::sockaddr_in addr);
        NetAddr();
        ::sockaddr_in getAddr();
        int getPort();
        std::string getIp();
        void setAddr(::sockaddr_in addr);
        std::string toString();

        private:
        ::sockaddr_in addr_;
    };
}
#endif