#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "../Thread.h"

using namespace base;

//使用放在User Server中的两个map成员来保证mfd的交互

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

void mkPts()
{
    for (int i = 0; i < 5; i++)
    {
    //创建master、slave对并解锁slave字符设备文件
    int mfd = open("/dev/ptmx", O_RDWR);
    grantpt(mfd);
    unlockpt(mfd);
    //查询并在控制台打印slave文件位置
    fprintf(stderr,"%s\n",ptsname(mfd));
    char **argv = (char**)malloc(sizeof(char*) * 2);
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
    /* else
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
    }*/
    }

    return;
}

int main(void)
{
    printf("Hello\n");
    Thread th(std::bind(mkPts));
    //Thread th2(std::bind(mkPts));
    //Thread th3(std::bind(mkPts));
    th.start();
    //th2.start();
    //th3.start();
    while(1)
    {

    }
}