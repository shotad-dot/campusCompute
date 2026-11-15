#include <stdio.h>
#include <string.h>
#include "campus.h"

#define MAX_SEGMENTS 33000

typedef struct
{
    int job_id, start, end;
} segment_t;

static segment_t timeline[MAX_SEGMENTS];
static int segment_n;

static void add_segment(int id,int start,int end)
{
    if(segment_n<MAX_SEGMENTS)
    {
        timeline[segment_n].job_id=id;
        timeline[segment_n].start=start;
        timeline[segment_n].end=end;
        segment_n++;
    }
}

static int order_before(const job_t *a,const job_t *b)
{
    return a->arrival_time<b->arrival_time ||
          (a->arrival_time==b->arrival_time && a->job_id<b->job_id);
}

static void sort_arrivals(job_t a[],int n)
{
    for(int i=1;i<n;i++)
    {
        job_t t=a[i];
        int j=i-1;
        while(j>=0 && order_before(&t,&a[j]))
        {
            a[j+1]=a[j];
            j--;
        }
        a[j+1]=t;
    }
}

static void complete_job(job_t *j,int time)
{
    j->completion_time=time;
    j->turnaround_time=time-j->arrival_time;
    j->waiting_time=j->turnaround_time-j->burst_time;
    j->status=1;
}

static void run_nonpreemptive(job_t a[],int n,int priority_mode)
{
    int done=0,time=0;
    while(done<n)
    {
        int pick=-1;
        for(int i=0;i<n;i++)
        {
            if(a[i].status || a[i].arrival_time>time) continue;
            if(pick==-1 ||
               (priority_mode && (a[i].priority<a[pick].priority ||
                (a[i].priority==a[pick].priority && order_before(&a[i],&a[pick])))) ||
               (!priority_mode && order_before(&a[i],&a[pick])))
                pick=i;
        }
        if(pick==-1)
        {
            int next=-1;
            for(int i=0;i<n;i++)
                if(!a[i].status && (next==-1 || a[i].arrival_time<next))
                    next=a[i].arrival_time;
            add_segment(0,time,next);
            time=next;
            continue;
        }
        a[pick].start_time=time;
        add_segment(a[pick].job_id,time,time+a[pick].burst_time);
        time+=a[pick].burst_time;
        complete_job(&a[pick],time);
        done++;
    }
}

static void run_round_robin(job_t a[],int n,int quantum)
{
    int queue[MAX_JOBS],front=0,count=0,remaining[MAX_JOBS];
    int next=0,done=0,time=0;
    sort_arrivals(a,n);
    for(int i=0;i<n;i++) remaining[i]=a[i].burst_time;

    while(done<n)
    {
        // Newly arrived jobs enter the queue before a preempted job goes back.
        while(next<n && a[next].arrival_time<=time)
        {
            queue[(front+count)%MAX_JOBS]=next++;
            count++;
        }
        if(count==0)
        {
            add_segment(0,time,a[next].arrival_time);
            time=a[next].arrival_time;
            continue;
        }
        int i=queue[front];
        front=(front+1)%MAX_JOBS;
        count--;
        if(a[i].start_time==-1) a[i].start_time=time;
        int slice=remaining[i]<quantum ? remaining[i] : quantum;
        add_segment(a[i].job_id,time,time+slice);
        remaining[i]-=slice;
        time+=slice;
        while(next<n && a[next].arrival_time<=time)
        {
            queue[(front+count)%MAX_JOBS]=next++;
            count++;
        }
        if(remaining[i]>0)
        {
            queue[(front+count)%MAX_JOBS]=i;
            count++;
        }
        else
        {
            complete_job(&a[i],time);
            done++;
        }
    }
}

int run_schedule(const job_t jobs[],int n,const char *mode,int quantum,result_t *result)
{
    job_t a[MAX_JOBS];
    memcpy(a,jobs,(size_t)n*sizeof(job_t));
    segment_n=0;
    if(strcmp(mode,"fcfs")==0)
    {
        printf("\n=== FCFS SCHEDULING ===\n");
        printf("tie-breaking : earlier arrival, then lower job ID\n");
        run_nonpreemptive(a,n,0);
    }
    else if(strcmp(mode,"priority")==0)
    {
        printf("\n=== NON-PREEMPTIVE PRIORITY SCHEDULING ===\n");
        printf("priority 1 is highest; ties : earlier arrival, then lower job ID\n");
        run_nonpreemptive(a,n,1);
    }
    else if(strcmp(mode,"rr")==0 && quantum>0)
    {
        printf("\n=== ROUND ROBIN SCHEDULING ===\n");
        printf("time quantum : %d\n",quantum);
        printf("initial ready order : earlier arrival, then lower job ID\n");
        printf("new arrivals enter before the preempted job is requeued\n");
        run_round_robin(a,n,quantum);
    }
    else return 0;

    printf("execution timeline (start -> end) :\n");
    for(int i=0;i<segment_n;i++)
    {
        if(timeline[i].job_id)
            printf("  %d -> %d : J%d\n",timeline[i].start,timeline[i].end,timeline[i].job_id);
        else
            printf("  %d -> %d : IDLE\n",timeline[i].start,timeline[i].end);
    }
    double wait=0,turn=0;
    printf("\n%-6s %-8s %-6s %-8s %-9s %-8s %-8s %-10s\n",
           "Job","Arrival","Burst","Priority","Start","CT","TAT","WT");
    for(int i=0;i<n;i++)
    {
        printf("%-6d %-8d %-6d %-8d %-9d %-8d %-8d %-10d",
               a[i].job_id,a[i].arrival_time,a[i].burst_time,a[i].priority,
               a[i].start_time,a[i].completion_time,a[i].turnaround_time,a[i].waiting_time);
        printf("%s\n",a[i].status ? "COMPLETED" : "PENDING");
        wait+=a[i].waiting_time;
        turn+=a[i].turnaround_time;
    }
    result->avg_wait=wait/n;
    result->avg_turn=turn/n;
    result->finish_time=timeline[segment_n-1].end;
    printf("average waiting time : %.2f\n",result->avg_wait);
    printf("average turnaround time : %.2f\n",result->avg_turn);
    printf("all %d jobs completed; finish time : %d\n",n,result->finish_time);
    return 1;
}
