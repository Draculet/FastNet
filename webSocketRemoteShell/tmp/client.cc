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
    printf("Client Running\n");
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
    printf("Client Running\n");
    ret=connect(clientFd,(struct sockaddr *)&serveraddr,sizeof(serveraddr));
    //连接到服务器
    printf("ret %d\n", ret);
    if(ret!=0)
    {
    	close(clientFd);
    	printf("connect error:%s\n",strerror(errno));
        exit(-1);
	}
    printf("Client Alive\n");
    int pid = fork();
        if (pid == 0)
        {
            while(1)
            {
                char buf[1] = {0};
                if(read(clientFd,buf,1)>0)
                    write(1,buf,1);
                else
                    sleep(1);
            }
        }
        while(1)
        {
            char buf[1] = {0};
            if(read(0,buf,1)>0)
                write(clientFd,buf,1);
            else
                sleep(1);
        }
}