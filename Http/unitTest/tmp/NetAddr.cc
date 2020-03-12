#include "NetAddr.h"
#include <assert.h>

using namespace std;
using namespace net;


NetAddr::NetAddr(int fd, sockaddr_in addr)
{
    assert(fd_ != -1);
    addr_ = addr;
    fd_ = fd;
}

