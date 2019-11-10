#ifndef __TCP_STATE_H__
#define __TCP_STATE_H__

#include <netinet/tcp.h>
#include <stdio.h>
#include <string.h>

namespace net
{
    void getTcpInfo(int sockfd)
    {
        struct tcp_info tcpi;
        socklen_t len = sizeof(tcpi);
        bzero(&tcpi, len);
        int ret = ::getsockopt(sockfd, SOL_TCP, TCP_INFO, &tcpi, &len);
        if (ret < 0)
        {
            printf("Get TcpInfo Error\n");
        }
        else
        {
            if (tcpi.tcpi_state == TCP_LISTEN)
                printf("LISTERN\n");
            else if (tcpi.tcpi_state == TCP_TIME_WAIT)
                printf("TIME_WAIT\n");
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
}

#endif