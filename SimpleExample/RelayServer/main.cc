#include "RelayServer.h"
using namespace std;

/* TODO 通过JSON配置转发的远程主机信息 */
/* 将转发到的远程主机的ip,port */
string ip = "127.0.0.1";
int port = 8833;

int main(void)
{
    RelayServer serv(8832);
    serv.start();
}