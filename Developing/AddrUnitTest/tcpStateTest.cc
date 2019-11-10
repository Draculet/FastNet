#include "TcpState.h"
#include "Socket.h"

using namespace net;

int main(void)
{
    printf("ok\n");
    Socket s = Socket(9000);
    getTcpInfo(s.getFd());
    s.listen();
    Socket s1 = Socket(9000);
    getTcpInfo(s1.getFd());
}