#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "campus.h"

int read_jobs(const char *file_name, job_t jobs[], int *n)
{
    FILE *fp=fopen(file_name,"r");
    if(fp==NULL)
    {
        perror("could not open job file");
        return 0;
    }

    char line[256], extra;
    int line_no=0;
    *n=0;
    while(fgets(line,sizeof(line),fp)!=NULL)
    {
        line_no++;
        if(strchr(line,'\n')==NULL && strlen(line)==sizeof(line)-1)
        {
            fprintf(stderr,"invalid line %d : line is too long\n",line_no);
            fclose(fp);
            return 0;
        }

        char *p=line;
        while(isspace((unsigned char)*p)) p++;
        if(*p=='#' || *p=='\0') continue;

        int id,arr,burst,priority;
        // extra character check makes sure each line has exactly four integers.
        if(sscanf(p,"%d %d %d %d %c",&id,&arr,&burst,&priority,&extra)!=4)
        {
            fprintf(stderr,"invalid line %d : expected ID ARRIVAL BURST PRIORITY\n",line_no);
            fclose(fp);
            return 0;
        }
        if(id<=0 || arr<0 || arr>100000 || burst<=0 || burst>1000 || priority<1 || priority>100)
        {
            fprintf(stderr,"invalid line %d : ID>0, arrival 0..100000, burst 1..1000, priority 1..100\n",line_no);
            fclose(fp);
            return 0;
        }
        if(*n==MAX_JOBS)
        {
            fprintf(stderr,"too many jobs : maximum is %d\n",MAX_JOBS);
            fclose(fp);
            return 0;
        }
        for(int i=0;i<*n;i++)
            if(jobs[i].job_id==id)
            {
                fprintf(stderr,"invalid line %d : duplicate job ID %d\n",line_no,id);
                fclose(fp);
                return 0;
            }
        job_t j={0};
        j.job_id=id;
        j.arrival_time=arr;
        j.burst_time=burst;
        j.priority=priority;
        j.start_time=-1;
        jobs[(*n)++]=j;
    }
    if(ferror(fp))
    {
        perror("error reading job file");
        fclose(fp);
        return 0;
    }
    fclose(fp);
    if(*n==0)
    {
        fprintf(stderr,"job file is empty\n");
        return 0;
    }
    return 1;
}

void show_jobs(const job_t jobs[], int n)
{
    printf("JOB INPUT (%d jobs)\n",n);
    printf("%-7s %-9s %-7s %-9s %-9s\n","ID","Arrival","Burst","Priority","Status");
    for(int i=0;i<n;i++)
        printf("%-7d %-9d %-7d %-9d %-9s\n",jobs[i].job_id,
               jobs[i].arrival_time,jobs[i].burst_time,jobs[i].priority,"NEW");
}
