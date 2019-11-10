#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
 
#define SERVERIP "127.0.0.1"
#define SERVERPORT 9981
#define MAXBUFFER 1024


void getTcpInfo(int sockfd)
{
    struct tcp_info tcpi;
    socklen_t len = sizeof(tcpi);
    bzero(&tcpi, len);
    int ret = ::getsockopt(sockfd, SOL_TCP, TCP_INFO, &tcpi, &len);
	//printf("%d,%d,%d,%d\n",TCP_FIN_WAIT1, TCP_FIN_WAIT2, TCP_CLOSE, TCP_TIME_WAIT);
    if (ret < 0)
    {
        printf("Get Info Error\n");
    }
    else
    {
        if (tcpi.tcpi_state == TCP_LISTEN)
            printf("LISTERN\n");
        else if (tcpi.tcpi_state == TCP_FIN_WAIT1)
            printf("FIN_WAIT\n");
        else if (tcpi.tcpi_state == TCP_FIN_WAIT2)
            printf("FIN_WAIT2\n");
        else if (tcpi.tcpi_state == TCP_TIME_WAIT)
		{
            printf("TIME_WAIT\n");
			exit(-1);
		}
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

int main(int argc, char** argv)
{  
		int clientFd,ret;
    	struct sockaddr_in serveraddr;
    	char buf[MAXBUFFER];
    	clientFd=socket(AF_INET,SOCK_STREAM,0);//创建socket
		getTcpInfo(clientFd);
    	if(clientFd < 0)
    	{
        	printf("socket error:%s\n",strerror(errno));
    	}
    	bzero(&serveraddr,sizeof(serveraddr));
    	serveraddr.sin_family=AF_INET;
    	serveraddr.sin_port=htons(SERVERPORT);
    	inet_pton(AF_INET,SERVERIP,&serveraddr.sin_addr);
    	ret=connect(clientFd,(struct sockaddr *)&serveraddr,sizeof(serveraddr));
		getTcpInfo(clientFd);
    	//连接到服务器
    	if(ret!=0)
    	{
    	    close(clientFd);
    	    printf("connect error:%s\n",strerror(errno));
    	}

		while(1)
		{
			int ret = read(clientFd, buf, 512);
			getTcpInfo(clientFd);
			if (ret == 0)
			{
				printf("read return 0\n");
				sleep(1);
				//读到0之后(即收到服务端的fin,此时状态处于close_wait)
				//调用shutdown发送fin
				//::shutdown(clientFd, SHUT_RDWR);
				//不断获取状态信息
				while(1)
				{
					getTcpInfo(clientFd);
				}
				break;
			}
		}
		//sleep(10);
    	/* 
		while(1)
    	{
			char buf[10240];
			int ret = 0;
    		 
			int ret = read(clientFd, buf, 100);
			getTcpInfo(clientFd);
			//sleep(2);
			printf("read ret %d\n", ret);
			ret = read(clientFd, buf, 100);
			//sleep(2);
			printf("read ret %d\n", ret);
			ret = read(clientFd, buf, 100);
			//sleep(2);
			printf("read ret %d\n", ret);
			ret = read(clientFd, buf, 100);
			//sleep(2);
			printf("read ret %d\n", ret);
			ret = read(clientFd, buf, 100);
			//sleep(2);
			ret = read(clientFd, buf, 100);
			getTcpInfo(clientFd);
			printf("read ret %d\n", ret);
			//sleep(2);
			getTcpInfo(clientFd);
			ret = read(clientFd, buf, 100);
			printf("read ret %d\n", ret);
			getTcpInfo(clientFd);
			//ret = write(clientFd, buf, 100);
			//printf("write ret %d\n", ret);
			getTcpInfo(clientFd);
			perror("write");
			getTcpInfo(clientFd);
			*/
			
			
			//shutdown(clientFd, SHUT_WR);
			//printf("shutdown\n");
			//close(clientFd);
			//getTcpInfo(clientFd);
			//break;

}

