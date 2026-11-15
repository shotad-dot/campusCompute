#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include "campus.h"

int run_limits(void)
{
    printf("\n=== RLIMIT_NOFILE RESOURCE LIMIT ===\n");
    fflush(stdout);
    pid_t pid=fork();
    if(pid==-1) {perror("fork limit test");return 0;}
    if(pid==0)
    {
        struct rlimit lim;
        if(getrlimit(RLIMIT_NOFILE,&lim)==-1) {perror("getrlimit");_exit(2);}
        printf("original open-file soft limit : %llu\n",(unsigned long long)lim.rlim_cur);
        lim.rlim_cur=lim.rlim_cur<32 ? lim.rlim_cur : 32;
        if(lim.rlim_cur<8) {fprintf(stderr,"file limit too low for safe demonstration\n");_exit(3);}
        if(setrlimit(RLIMIT_NOFILE,&lim)==-1) {perror("setrlimit");_exit(4);}
        if(getrlimit(RLIMIT_NOFILE,&lim)==-1) {perror("verify limit");_exit(5);}
        printf("applied soft limit : %llu\n",(unsigned long long)lim.rlim_cur);
        int fds[64],count=0,hit_limit=0;
        while(count<64)
        {
            int fd=open("/dev/null",O_RDONLY);
            if(fd==-1)
            {
                if(errno==EMFILE)
                {
                    printf("observed effect : open failed with EMFILE (too many open files)\n");
                    hit_limit=1;
                }
                else perror("open /dev/null");
                break;
            }
            fds[count++]=fd;
        }
        printf("additional file descriptors opened : %d\n",count);
        for(int i=0;i<count;i++) close(fds[i]);
        printf("closed all descriptors opened by limit test\n");
        fflush(stdout);
        _exit(hit_limit ? 0 : 6);
    }
    int status;
    if(waitpid(pid,&status,0)==-1) {perror("waitpid limit test");return 0;}
    printf("resource-limit child collected; parent limits remain unchanged\n");
    return WIFEXITED(status) && WEXITSTATUS(status)==0;
}
