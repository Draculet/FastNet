#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Buffer.h"
#include <iostream>
#include "../Base/Mutex.h"
using namespace net;

struct node
{
    Mutex *m;
    Buffer *buf;
    int *epfd;
};

void *threadFunc(void *p)
{
    node *n = static_cast<node *>(p);
    Buffer *buf = n->buf;
    Mutex *m = n->m;
    int epfd = *(n->epfd);
    struct sockaddr_in serveraddr;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        perror("socket");
        exit(-1);
    }
    bzero(&serveraddr,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9981);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int ret=bind(listenfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr));
    if(ret != 0)
    {
        close(listenfd);
        printf("bind error:%s\n",strerror(errno));
        exit(-1);
    }
    epoll_event ev = {0};
    epoll_event actievs[30] = {0};
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
    listen(listenfd , 10);
    for ( ; ; )
    {
        int nfds = epoll_wait(epfd, actievs, 30 , -1);
        printf("epoll wait ret\n");
        for (int i = 0; i < nfds; i++)
        {
            if (actievs[i].data.fd == listenfd)
            {
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                bzero(&clientaddr,sizeof(clientaddr));
                int connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &len);
                char ip[15] = {0};
                printf("%s 连接到服务器,端口号 %d\n",inet_ntop(AF_INET, &clientaddr.sin_addr, ip ,sizeof(ip)),ntohs(clientaddr.sin_port));
                epoll_event ev = {0};
                ev.events = EPOLLIN;
                ev.data.fd = connfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd , &ev);
            }
            else if (actievs[i].events & EPOLLIN)
            {
                size_t ret;
                sleep(3);
              //{//mutex
               //MutexGuard mutex(*m);
                if ((ret = buf->readFd(actievs[i].data.fd)) != 0)
                {
                    printf("ReadFd ret %d\n", ret);
                    std::string tmp = "\n=================From fd===============\n";
                    buf->append(tmp.c_str(), tmp.size());
                }
                else if (ret == -1)
                    printf("Error ReadFd\n");
                else
                {
                    close(actievs[i].data.fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, actievs[i].data.fd, NULL);
                }
              //}//mutex
            }
        }
    }
}


int main(void)
{
    Mutex m;
    Buffer buf;
    node n;
    n.m = &m;
    n.buf = &buf;
    int epfd = epoll_create(10);
    n.epfd = &epfd;
    pthread_t pid;
    int res = pthread_create(&pid, NULL, threadFunc, &n);
    epoll_event ev = {0};
    ev.events = 0;
    ev.data.fd = STDIN_FILENO;
    epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
    while(1)
    {
        sleep(1);
        //MutexGuard mutex(m);
        //std::cout << "========================Main=====================" << std::endl;
        //std::cout << buf.preViewAsString(buf.readable()) << std::endl;
        //std::cout << "========================Main END=====================" << std::endl;
        sleep(15);
        epoll_event ev = {0};
        ev.events |= EPOLLIN;
        ev.data.fd = STDIN_FILENO;
        epoll_ctl(epfd, EPOLL_CTL_MOD, STDIN_FILENO, &ev);
    }
}