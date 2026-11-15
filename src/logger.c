#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>
#include "campus.h"

#define CAPACITY 4
#define EVENT_COUNT 10

typedef struct
{
    int buffer[CAPACITY];
    int front,back,count,closed,failed;
    pthread_mutex_t mutex;
    pthread_cond_t not_full,not_empty;
} queue_t;

static queue_t q;

static void *produce(void *unused)
{
    (void)unused;
    for(int event=1;event<=EVENT_COUNT;event++)
    {
        pthread_mutex_lock(&q.mutex);
        while(q.count==CAPACITY && !q.failed)
            pthread_cond_wait(&q.not_full,&q.mutex);
        if(q.failed)
        {
            pthread_mutex_unlock(&q.mutex);
            break;
        }
        q.buffer[q.back]=event;
        q.back=(q.back+1)%CAPACITY;
        q.count++;
        printf("producer : queued event %d (queue size %d/%d)\n",event,q.count,CAPACITY);
        pthread_cond_signal(&q.not_empty);
        pthread_mutex_unlock(&q.mutex);
    }
    pthread_mutex_lock(&q.mutex);
    q.closed=1;
    pthread_cond_broadcast(&q.not_empty);
    pthread_mutex_unlock(&q.mutex);
    printf("producer : thread finished\n");
    return NULL;
}

static void *consume(void *unused)
{
    (void)unused;
    FILE *fp=fopen("logs/activity.log","a");
    if(fp==NULL)
    {
        perror("logger file");
        pthread_mutex_lock(&q.mutex);
        q.failed=1;
        pthread_cond_broadcast(&q.not_full);
        pthread_mutex_unlock(&q.mutex);
        return NULL;
    }
    while(1)
    {
        pthread_mutex_lock(&q.mutex);
        while(q.count==0 && !q.closed)
            pthread_cond_wait(&q.not_empty,&q.mutex);
        if(q.count==0 && q.closed)
        {
            pthread_mutex_unlock(&q.mutex);
            break;
        }
        int event=q.buffer[q.front];
        q.front=(q.front+1)%CAPACITY;
        q.count--;
        printf("consumer : removed event %d (queue size %d/%d)\n",event,q.count,CAPACITY);
        pthread_cond_signal(&q.not_full);
        pthread_mutex_unlock(&q.mutex);
        if(fprintf(fp,"event %02d : processed by consumer thread\n",event)<0)
        {
            perror("writing log");
            pthread_mutex_lock(&q.mutex);
            q.failed=1;
            pthread_cond_broadcast(&q.not_full);
            pthread_mutex_unlock(&q.mutex);
            break;
        }
    }
    if(fclose(fp)!=0)
    {
        perror("closing log");
        pthread_mutex_lock(&q.mutex);
        q.failed=1;
        pthread_mutex_unlock(&q.mutex);
    }
    printf("consumer : thread finished\n");
    return NULL;
}

int run_logger(void)
{
    if(mkdir("logs",0700)==-1 && errno!=EEXIST)
    {
        perror("creating logs directory");
        return 0;
    }
    memset(&q,0,sizeof(q));
    int err=pthread_mutex_init(&q.mutex,NULL);
    if(err) {fprintf(stderr,"mutex init : %s\n",strerror(err));return 0;}
    err=pthread_cond_init(&q.not_full,NULL);
    if(err) {fprintf(stderr,"condition init : %s\n",strerror(err));pthread_mutex_destroy(&q.mutex);return 0;}
    err=pthread_cond_init(&q.not_empty,NULL);
    if(err) {fprintf(stderr,"condition init : %s\n",strerror(err));pthread_cond_destroy(&q.not_full);pthread_mutex_destroy(&q.mutex);return 0;}

    pthread_t producer,consumer;
    printf("\n=== BOUNDED PRODUCER-CONSUMER LOGGER ===\n");
    printf("queue capacity : %d; mutex + not_full/not_empty conditions\n",CAPACITY);
    err=pthread_create(&consumer,NULL,consume,NULL);
    if(err)
    {
        fprintf(stderr,"consumer creation : %s\n",strerror(err));
        pthread_cond_destroy(&q.not_empty);pthread_cond_destroy(&q.not_full);
        pthread_mutex_destroy(&q.mutex);
        return 0;
    }
    err=pthread_create(&producer,NULL,produce,NULL);
    if(err)
    {
        fprintf(stderr,"producer creation : %s\n",strerror(err));
        pthread_mutex_lock(&q.mutex);q.closed=1;pthread_cond_broadcast(&q.not_empty);pthread_mutex_unlock(&q.mutex);
        pthread_join(consumer,NULL);
        pthread_cond_destroy(&q.not_empty);pthread_cond_destroy(&q.not_full);
        pthread_mutex_destroy(&q.mutex);
        return 0;
    }
    pthread_join(producer,NULL);
    pthread_join(consumer,NULL);
    int failed=q.failed;
    pthread_cond_destroy(&q.not_empty);
    pthread_cond_destroy(&q.not_full);
    pthread_mutex_destroy(&q.mutex);
    printf("pthread_join verified : both threads terminated\n");
    printf("pthread synchronization objects destroyed\n");
    printf("generated log : logs/activity.log\n");
    return !failed;
}
