#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "campus.h"

typedef struct
{
    int number;
    char message[120];
} shared_t;

static int retry_sem_wait(sem_t *sem)
{
    int rc;
    do {rc=sem_wait(sem);} while(rc==-1 && errno==EINTR);
    return rc;
}

int run_ipc(void)
{
    char shm_name[80],ready_name[80],reply_name[80];
    // Each execution uses its own PID in the name, so we never unlink someone else's IPC object.
    snprintf(shm_name,sizeof(shm_name),"/campuscompute_%ld",(long)getpid());
    snprintf(ready_name,sizeof(ready_name),"/campuscompute_ready_%ld",(long)getpid());
    snprintf(reply_name,sizeof(reply_name),"/campuscompute_reply_%ld",(long)getpid());
    printf("\n=== POSIX SHARED MEMORY AND NAMED SEMAPHORES ===\n");
    printf("owned shared-memory name : %s\n",shm_name);
    int ok=0,fd=-1,child_started=0;
    shared_t *data=MAP_FAILED;
    sem_t *ready=SEM_FAILED,*reply=SEM_FAILED;
    pid_t pid=-1;
    int status=0;
    fd=shm_open(shm_name,O_CREAT|O_EXCL|O_RDWR,0600);
    if(fd==-1) {perror("shm_open");goto finish;}
    if(ftruncate(fd,sizeof(shared_t))==-1) {perror("ftruncate");goto finish;}
    data=mmap(NULL,sizeof(shared_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if(data==MAP_FAILED) {perror("mmap");goto finish;}
    ready=sem_open(ready_name,O_CREAT|O_EXCL,0600,0);
    if(ready==SEM_FAILED) {perror("sem_open ready");goto finish;}
    reply=sem_open(reply_name,O_CREAT|O_EXCL,0600,0);
    if(reply==SEM_FAILED) {perror("sem_open reply");goto finish;}

    data->number=42;
    snprintf(data->message,sizeof(data->message),"task posted by parent");
    fflush(stdout);
    pid=fork();
    if(pid==-1) {perror("fork");goto finish;}
    if(pid==0)
    {
        if(retry_sem_wait(ready)==-1) _exit(3);
        printf("child PID %ld : received %d, %s\n",(long)getpid(),data->number,data->message);
        data->number*=2;
        snprintf(data->message,sizeof(data->message),"result written by child");
        if(sem_post(reply)==-1) _exit(4);
        printf("child : posted reply semaphore\n");
        fflush(stdout);
        _exit(0);
    }
    child_started=1;
    printf("parent PID %ld : wrote value 42 into shared memory\n",(long)getpid());
    if(sem_post(ready)==-1) {perror("sem_post ready");goto finish;}
    printf("parent : posted ready semaphore; child can read\n");
    if(retry_sem_wait(reply)==-1) {perror("sem_wait reply");goto finish;}
    printf("parent : received reply semaphore\n");
    printf("parent : shared result %d, %s\n",data->number,data->message);
    if(waitpid(pid,&status,0)==-1) {perror("waitpid");goto finish;}
    child_started=0;
    if(!WIFEXITED(status) || WEXITSTATUS(status)!=0)
    {
        fprintf(stderr,"IPC child did not finish normally\n");
        goto finish;
    }
    printf("waitpid verified : IPC child exited with status 0\n");
    ok=1;

finish:
    if(child_started)
    {
        // An error occurred after fork; unblock a possibly waiting child before collecting it.
        if(ready!=SEM_FAILED) sem_post(ready);
        if(reply!=SEM_FAILED) sem_post(reply);
        waitpid(pid,&status,0);
    }
    if(data!=MAP_FAILED && munmap(data,sizeof(shared_t))==-1) {perror("munmap");ok=0;}
    if(fd!=-1 && close(fd)==-1) {perror("close shm fd");ok=0;}
    if(ready!=SEM_FAILED && sem_close(ready)==-1) {perror("sem_close ready");ok=0;}
    if(reply!=SEM_FAILED && sem_close(reply)==-1) {perror("sem_close reply");ok=0;}
    if(reply!=SEM_FAILED && sem_unlink(reply_name)==-1) {perror("sem_unlink reply");ok=0;}
    if(ready!=SEM_FAILED && sem_unlink(ready_name)==-1) {perror("sem_unlink ready");ok=0;}
    if(fd!=-1 && shm_unlink(shm_name)==-1) {perror("shm_unlink");ok=0;}

    // Check the exact names created by this run, without touching any other IPC object.
    if(fd!=-1)
    {
        int probe=shm_open(shm_name,O_RDONLY,0);
        if(probe!=-1) {fprintf(stderr,"shared memory still exists\n");close(probe);ok=0;}
        else if(errno!=ENOENT) {perror("verify shm cleanup");ok=0;}
        else printf("verified : shared-memory name no longer exists\n");
    }
    if(ready!=SEM_FAILED)
    {
        sem_t *probe=sem_open(ready_name,0);
        if(probe!=SEM_FAILED) {fprintf(stderr,"ready semaphore still exists\n");sem_close(probe);ok=0;}
        else if(errno!=ENOENT) {perror("verify ready cleanup");ok=0;}
        else printf("verified : ready semaphore name no longer exists\n");
    }
    if(reply!=SEM_FAILED)
    {
        sem_t *probe=sem_open(reply_name,0);
        if(probe!=SEM_FAILED) {fprintf(stderr,"reply semaphore still exists\n");sem_close(probe);ok=0;}
        else if(errno!=ENOENT) {perror("verify reply cleanup");ok=0;}
        else printf("verified : reply semaphore name no longer exists\n");
    }
    printf("IPC cleanup status : %s\n",ok ? "PASS" : "FAIL");
    return ok;
}
