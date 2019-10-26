#ifndef __NET_NETADDR_H__
#define __NET_NETADDR_H__
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

using namespace std;

    class NetAddr
    {
        public:
        sockaddr_in getAddr()
        {
            return addr_;
        }

        int getPort()
        {
            return addr_.sin_port;
        }

        string getIp()
        {
            char ip[64] = {0}
            return string(inet_ntop(AF_INET, &addr_.sin_addr, ip, sizeof(ip)));
        }

        int getFd()
        {
            return fd_;
        }

        private:
        sockaddr_in addr_;
        int fd_;
    };
#endif