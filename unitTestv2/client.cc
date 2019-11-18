#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXBUFFER 1024 * 1024 * 4
int main(void)
{
    int clientFd,ret;
    struct sockaddr_in serveraddr;
    char buf[MAXBUFFER];
    clientFd = socket(AF_INET,SOCK_STREAM,0);//创建socket
    if(clientFd < 0)
    {
        printf("socket error:%s\n",strerror(errno));
    }
    bzero(&serveraddr,sizeof(serveraddr));
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(8832);
    inet_pton(AF_INET,"127.0.0.1",&serveraddr.sin_addr);
    ret=connect(clientFd,(struct sockaddr *)&serveraddr,sizeof(serveraddr));
    //连接到服务器
    if(ret!=0)
    {
    	close(clientFd);
    	printf("connect error:%s\n",strerror(errno));
	}
    int i = 1;
    while(1)
    {
        int ret = write(clientFd, buf, MAXBUFFER/i);
        perror("write");
        printf("send %d bytes\n", ret);
        sleep(2);
        ret = read(clientFd, buf, MAXBUFFER);
        printf("recv %d bytes\n", ret);
        //i-= 100;
    }
}