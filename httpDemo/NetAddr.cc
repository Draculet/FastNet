#include "NetAddr.h"
#include <assert.h>

using namespace std;
using namespace net;

NetAddr::NetAddr(int port)
{
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    assert(fd_ != -1);//TODO 错误处理
    bzero(&addr_,sizeof(&addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    int ret = bind(fd_, (struct sockaddr *)&addr_, sizeof(addr_));//绑定IP和端口
    assert(ret != -1);//TODO 错误处理
}

NetAddr::NetAddr(int fd, sockaddr_in addr)
{
    assert(fd_ != -1);
    addr_ = addr;
    fd_ = fd;
}

