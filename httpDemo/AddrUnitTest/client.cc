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
#define MAXBUFFER 256

void *client_echo(void *);

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
        if (tcpi.tcpi_state == TCP_LISTEN)
            printf("LISTERN\n");
        else if (tcpi.tcpi_state == TCP_FIN_WAIT1)
            printf("FIN_WAIT\n");
        else if (tcpi.tcpi_state == TCP_FIN_WAIT2)
            printf("FIN_WAIT2\n");
        else if (tcpi.tcpi_state == TCP_TIME_WAIT)
            printf("TIME_WAIT\n");
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
	for (int i = 0; i < 2; i++)
	{
    	pthread_t pid;
    	pthread_create(&pid,NULL,&client_echo, NULL);
    }
    //pthread_mutex_lock(&mutex);
    //pthread_cond_broadcast(&condition);
    //pthread_mutex_unlock(&mutex);
    
    while(1)
    {
    	
    }
}

void *client_echo(void *)
{
	//int count = 0;
	//pthread_mutex_lock(&mutex);
    //pthread_cond_wait(&condition, &mutex);
    //pthread_mutex_unlock(&mutex);
	if (1)
	{
		int clientFd,ret;
    	struct sockaddr_in serveraddr;
    	char buf[MAXBUFFER];
    	clientFd=socket(AF_INET,SOCK_STREAM,0);//创建socket
    	if(clientFd < 0)
    	{
        	printf("socket error:%s\n",strerror(errno));
    	}
    	bzero(&serveraddr,sizeof(serveraddr));
    	serveraddr.sin_family=AF_INET;
    	serveraddr.sin_port=htons(SERVERPORT);
    	inet_pton(AF_INET,SERVERIP,&serveraddr.sin_addr);
    	ret=connect(clientFd,(struct sockaddr *)&serveraddr,sizeof(serveraddr));
    	//连接到服务器
    	if(ret!=0)
    	{
    	    close(clientFd);
    	    printf("connect error:%s\n",strerror(errno));
    	}
		//sleep(10);
    	while(1)
    	{
			char buf[10240];
			int ret = 0;
    		/* 
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
			while(1)
			{
				int ret = read(clientFd, buf, 10000);
				if (ret == 0)
				{
					::shutdown(clientFd, SHUT_RD);
                	::shutdown(clientFd, SHUT_WR);
					return NULL;
				}
			}
			
			//shutdown(clientFd, SHUT_WR);
			//printf("shutdown\n");
			//close(clientFd);
			//getTcpInfo(clientFd);
			//break;
    	}
    }
    /*
    while(1)
    {
        bzero(buf,sizeof(buf));
        write(clientFd,buf,sizeof(buf));//写数据
        bzero(buf,sizeof(buf));
        read(clientFd,buf,sizeof(buf));//读数据
        printf("echo:%s\n",buf);
    }
    close(clientFd);
    */
    return NULL;
}
