#ifndef __NET_NETADDR_H__
#define __NET_NETADDR_H__
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fcntl.h>

using namespace std;

    class NetAddr
    {
        public:
        NetAddr(sockaddr_in addr):
            addr_(addr)
        {

        }

        NetAddr()
        {
            bzero(&addr_, sizeof(addr_));
        }

        sockaddr_in getAddr()
        {
            return addr_;
        }

        int getPort()
        {
            return ntohs(addr_.sin_port);
        }

        string getIp()
        {
            char ip[64] = {0};
            return string(inet_ntop(AF_INET, &addr_.sin_addr, ip, sizeof(ip)));
        }

        void setAddr(sockaddr_in addr)
        {
            addr_ = addr;
        }

        string toString()
        {
            char buf[128] = {0};
            sprintf(buf, "%d", getPort());
            return getIp() + ":" + string(buf);
        }

        private:
        sockaddr_in addr_;
    };
#endif