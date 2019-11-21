#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "../Thread.h"

using namespace base;

void writeProcess(int mfd)
{
    char buf[1] = {'\0'};
        while(1)
        {
            if(read(0,buf,1)>0)
                write(mfd,buf,1);
            else
                sleep(1);
        }
}

int main(int argc, char *argv[])
{
    //创建master、slave对并解锁slave字符设备文件
    int mfd = open("/dev/ptmx", O_RDWR);
    grantpt(mfd);
    unlockpt(mfd);
    //查询并在控制台打印slave文件位置
    fprintf(stderr,"%s\n",ptsname(mfd));
    argv[1] = ptsname(mfd);
    argv[0] = "/home/homulia/fastNet/remoteShell/loginExe";
    //printf("%s\n", argv[1]);
    int pid=fork();//分为两个进程
    //相当于对master的读出
    if(pid == 0)
    {
        //printf("Son\n");
        execv("/home/homulia/fastNet/remoteShell/loginExe", argv);
    }
    //相当于对master的写入,此端与nc发送数据到本端的socket相连
    else
    {
        Thread th(std::bind(writeProcess, mfd));
        th.start();
        //printf("Father\n");
        char buf[1] = {'\0'};
        while(1)
        {
            if(read(mfd,buf,1)>0)
                write(1,buf,1);
            else
                sleep(1);
        }
    }

    return 0;
}