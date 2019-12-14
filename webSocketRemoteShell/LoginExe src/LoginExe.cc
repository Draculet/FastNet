#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <signal.h>

using namespace std;
int main(int argc, char *argv[])
{
    //if (argc != 2)
    //{
    //    printf("useage ./fileOutput /dev/pts/xx \n");
    //    return -1;
    //}
    printf("In LoginExe :%s\n", argv[1]);
    int fd = dup(STDOUT_FILENO);
    int ffd = open(argv[1], O_RDWR);
    
    dup2(ffd, 1);
    dup2(ffd, 2);
    dup2(ffd, 0);
    //printf("dasdas\n");
    /* for (int i = 0; i < argc; i++)
    {
        printf("%s\n", argv[i]);
    }
    */
    while (1)
    {
        argv[1] = 0;
        argv[0] = "/bin/login";
        int pid = fork();
        if (pid == 0)
        {
            prctl(PR_SET_PDEATHSIG, SIGKILL);
            int ret = execv("/bin/login", argv);
            if (ret < 0)
                exit(-1);
        }
        else
        {
            int ret = wait(nullptr);
        }
        //printf("Process %d Return\n", ret);
    }
}