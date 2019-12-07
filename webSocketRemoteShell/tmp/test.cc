#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int main(int argc, char *argv[])
{
    //创建master、slave对并解锁slave字符设备文件
    argv[1] = "/dev/pts/3";
    argv[0] = "";
    //argv[0] = "loginExe";
    execv("loginExe", argv);
}