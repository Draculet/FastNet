#include "NetAddr.h"
#include "Socket.h"

using namespace std;

void getTcpInfo(int sockfd)
{
    struct tcp_info tcpi;
    socklen_t len = sizeof(tcpi);
    bzero(&tcpi, len);
    int ret = ::getsockopt(sockfd, SOL_TCP, TCP_INFO, &tcpi, &len);
    if (ret < 0)
    {
        printf("Get Info Error\n");
    }
    else
    {
        printf("state num: %d\n", tcpi.tcpi_state);
        if (tcpi.tcpi_state == TCP_LISTEN)
            printf("LISTERN\n");
        else if (tcpi.tcpi_state == TCP_TIME_WAIT)
        {
            printf("TIME_WAIT\n");
            printf("TIME_WAIT\n");
            printf("TIME_WAIT\n");
            printf("TIME_WAIT\n");
            exit(-1);
        }
        else if (tcpi.tcpi_state == TCP_FIN_WAIT1)
            printf("FIN_WAIT\n");
        else if (tcpi.tcpi_state == TCP_FIN_WAIT2)
            printf("FIN_WAIT2\n");
        else if (tcpi.tcpi_state == TCP_ESTABLISHED)
            printf("ESTABLISHED\n");
        else if (tcpi.tcpi_state == TCP_CLOSE_WAIT)
            printf("CLOSE_WAIT\n");
        else if (tcpi.tcpi_state == TCP_LAST_ACK)
            printf("LAST_ACK\n");
        else if (tcpi.tcpi_state == TCP_CLOSE)
            printf("CLOSED\n");
        else if (tcpi.tcpi_state == TCP_SYN_RECV)
            printf("SYN_RCVD\n");
        else if (tcpi.tcpi_state == TCP_SYN_SENT)
            printf("SYN_SEND\n");
        else if (tcpi.tcpi_state == TCP_CLOSING)
            printf("CLOSING\n");
        else
            printf("ERROR_STATE\n");
    }
}

int main(void)
{
    //Socket soc("192.168.42.146", 9981);
    Socket soc(9981);
    int optval = 1;
    ::setsockopt(soc.getFd(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    optval = 1;
    ::setsockopt(soc.getFd(), SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    //Socket soc2("127.0.0.2", 9981);
    soc.listen();
    
    getTcpInfo(soc.getFd());
    //soc2.listen();
    //Socket *ns;
    cout << soc.getAddr()->getIp() << ":" << soc.getAddr()->getPort() << endl;
    while(1)
    {
        NetAddr addr;
        int fd = soc.accept(&addr);
        cout << "Accept new connection from " << soc.getAddr()->getIp() << ":" << soc.getAddr()->getPort() << endl;
        //fd2 = soc2.accept(&addr);
        //cout << "Accept new connection from " << soc.getAddr()->getIp() << ":" << soc.getAddr()->getPort() << endl;
        if (fd < 0)
        {
            sleep(5);
            continue;
        }
        Socket *ns = new Socket(fd, addr);
        getTcpInfo(ns->getFd());
        cout << "Accept new connection from " << ns->getAddr()->getIp() << ":" << ns->getAddr()->getPort() << endl;
        char buf[512] = {'h','e','l','l','o'};
        int ret;
        for (int i = 0; i < 10; i++)
        {
            ret = write(ns->getFd(), buf, 512);
            if (ret < 0)
            {
                perror("write");
            }
            else
            {
                printf("send %d bytes\n", ret);
            }
        }
        ::shutdown(ns->getFd(), SHUT_RD);
        ::shutdown(ns->getFd(), SHUT_WR);
        for (int i = 0; i < 1000; i++)
        {
            getTcpInfo(ns->getFd());
            if (i == 1)
            {   //close(ns->getFd());
                
            }
        }
        printf("close socket\n");
        //sleep(5);
    }
}