#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<termios.h>
#include<sys/ioctl.h>

int main(int argc, char *argv[])
{
    int old=open("/dev/tty",O_RDWR);  //打开当前控制终端
    ioctl(old, TIOCNOTTY);  //放弃当前控制终端
  
    //根据"man 2 setsid"的说明，调用setsid的进程不能是进程组组长（从bash中运行的命令是组长），故fork出一个子进程，让组长结束，子进程脱离进程组成为新的会话组长
    int pid=fork();
    if(pid==0){
        setsid();  //子进程成为会话组长
        perror("setsid");  //显示setsid是否成功
        ioctl(0, TIOCSCTTY, 0);  //这时可以设置新的控制终端了，设置控制终端为stdin
        execv("/bin/login", argv);  //把当前进程刷成login
    }
    return 0;
}