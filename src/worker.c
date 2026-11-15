#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "campus.h"

static int test_worker(int simulated_failure)
{
    fflush(stdout);
    pid_t pid=fork();
    if(pid==-1) {perror("fork worker");return 0;}
    if(pid==0)
    {
        printf("child PID %ld : %s\n",(long)getpid(),simulated_failure ?
               "controlled SIMULATED failure (exit code 7)" : "work completed normally");
        fflush(stdout);
        _exit(simulated_failure ? 7 : 0);
    }
    int status;
    pid_t result;
    do {result=waitpid(pid,&status,0);} while(result==-1 && errno==EINTR);
    if(result==-1) {perror("waitpid worker");return 0;}
    printf("parent : collected child PID %ld using waitpid\n",(long)result);
    if(WIFEXITED(status))
    {
        int code=WEXITSTATUS(status);
        printf("parent : child exited with code %d (%s)\n",code,
               code==0 ? "NORMAL COMPLETION" : "SIMULATED FAILURE DETECTED");
        return simulated_failure ? code==7 : code==0;
    }
    if(WIFSIGNALED(status)) printf("parent : child terminated by signal %d\n",WTERMSIG(status));
    return 0;
}

int run_workers(void)
{
    printf("\n=== WORKER PROCESS : NORMAL COMPLETION ===\n");
    int a=test_worker(0);
    printf("\n=== WORKER PROCESS : CONTROLLED SIMULATED FAILURE ===\n");
    int b=test_worker(1);
    printf("worker cleanup : both children collected with waitpid\n");
    return a && b;
}
