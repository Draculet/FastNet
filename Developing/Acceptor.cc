#include "Acceptor.h"
#include "NetAddr.h"

using namespace std;
using namespace net;
using namespace base;

Acceptor::Acceptor(Eventloop *loop, int port):
    addr_(port),
    loop_(loop),
    acceptChannel_(addr_.getFd(), loop)
{
    
}

void Acceptor::listen()
{
    ::listen(addr_.getFd(), kinitListen);
    //必须先由main设置回调
    acceptChannel_.enableRead();
}
