#include <sys/uio.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    char arr[10];
    char arr2[10];
    memset(arr, 0, 10);
    memset(arr2, 0, 10);
    struct iovec vec[2];
    vec[0].iov_base = arr;
    vec[0].iov_len = 9;
    vec[1].iov_base = arr2;
    vec[1].iov_len = 9;
    int iovcnt = 2;
    while (1)
    {
    int n = readv(STDIN_FILENO, vec, iovcnt);
    printf("ret %d\n", n);
    printf("data:%s\n%s\n", arr, arr2);
    memset(arr, 0, 10);
    memset(arr2, 0, 10);
    }
}