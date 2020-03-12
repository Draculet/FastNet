#include "../include/NetAddr.h"

using namespace std;
using namespace net;

    NetAddr::NetAddr(sockaddr_in addr):
        addr_(addr)
    {

    }

    NetAddr::NetAddr()
    {
        bzero(&addr_, sizeof(addr_));
    }

    sockaddr_in NetAddr::getAddr()
    {
        return addr_;
    }

    int NetAddr::getPort()
    {
        return ntohs(addr_.sin_port);
    }

    string NetAddr::getIp()
    {
        char ip[64] = {0};
        return string(inet_ntop(AF_INET, &addr_.sin_addr, ip, sizeof(ip)));
    }

    void NetAddr::setAddr(sockaddr_in addr)
    {
        addr_ = addr;
    }

    string NetAddr::toString()
    {
        char buf[128] = {0};
        sprintf(buf, "%d", getPort());
        return getIp() + ":" + string(buf);
    }