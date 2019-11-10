#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <string.h>

using namespace std;

void getTcpInfo(int sockfd)
{
    struct tcp_info tcpi;
    socklen_t len = sizeof(tcp_info);
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
        int fd = ::socket(PF_INET, SOCK_STREAM, 0);
        getTcpInfo(fd);
        sockaddr_in servaddr;
        bzero(&servaddr,sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(9981);
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        int ret = ::bind(fd, (sockaddr *)&servaddr, sizeof(servaddr));
        if (ret < 0)
        {
            perror("bind");
            exit(-1);
        }
        ret = ::listen(fd, 64);
        if (ret < 0)
        {
            perror("listen");
        }
        getTcpInfo(fd);
        sockaddr_in cli_addr;
        socklen_t len = sizeof(cli_addr);
        int connfd = ::accept4(fd, (sockaddr *)&cli_addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        getTcpInfo(connfd);
        if (connfd < 0)
        {
            //非阻塞accept,只输出错误信息
            perror("accept4");
            //TODO 判创建失败错,日志输出
            return -1;
        }
        char buf[512] = {0};
        //写一点data
        ret = write(connfd, buf, 512);
        if (ret < 0)
        {
            perror("write");
        }
        else
        {
            printf("send %d bytes\n", ret);
        }
        sleep(1);
        //直接关闭
        //close会释放文件描述符,造成获取tcpinfo失败
        //shutdown之后会发送fin并进入fin_wait1
        //在接受到对方的ack后进入fin_wait2
        ::shutdown(connfd, SHUT_RDWR);
        //如果接受到对方发的fin之后理应进入time_wait状态
        //但下面打印的信息显示fin_wait2之后直接进入了close状态
        printf("close socket\n");
        getTcpInfo(connfd);
        //打印获取tcp连接状态
        for (;;)
        {
            getTcpInfo(connfd);
        }
        //sleep(5);
}