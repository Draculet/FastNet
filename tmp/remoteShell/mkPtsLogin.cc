#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

/* Chown the slave to the calling user.  */
extern int grantpt (int __fd) __THROW;

/* Release an internal lock so the slave can be opened.
   Call after grantpt().  */
extern int unlockpt (int __fd) __THROW;

/* Return the pathname of the pseudo terminal slave associated with
   the master FD is open on, or NULL on errors.
   The returned storage is good until the next call to this function.  */
extern char *ptsname (int __fd) __THROW __wur;

char buf[1]={'\0'};  //创建缓冲区，这里只需要大小为1字节
int main()
{
    //创建master、slave对并解锁slave字符设备文件
    int mfd = open("/dev/ptmx", O_RDWR);
    grantpt(mfd);
    unlockpt(mfd);
    //查询并在控制台打印slave文件位置
    fprintf(stderr,"%s\n",ptsname(mfd));

    int pid=fork();//分为两个进程
    //相当于对master的读出
    if(pid)//父进程从master读字节，并写入标准输出中
    {
        while(1)
        {
            if(read(mfd,buf,1)>0)
                write(1,buf,1);
            else
                sleep(1);
        }
    }
    //相当于对master的写入,此端与nc发送数据到本端的socket相连
    else//子进程从标准输入读字节，并写入master中
    {
        while(1)
        {
            if(read(0,buf,1)>0)
                write(mfd,buf,1);
            else
                sleep(1);
        }
    }

    return 0;
}