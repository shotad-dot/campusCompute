#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "campus.h"

void show_help(const char *program)
{
    printf("CampusCompute - Operating Systems mini-project\n");
    printf("Usage: %s <command> [jobs.txt] [quantum]\n\n",program);
    printf("  help                    show all commands\n");
    printf("  jobs       [file]       validate and display jobs\n");
    printf("  fcfs       [file]       FCFS and metrics\n");
    printf("  rr         [file] [q]   Round Robin, default quantum 2\n");
    printf("  priority   [file]       non-preemptive priority\n");
    printf("  compare    [file]       compare all algorithms on the same jobs\n");
    printf("  logger                  producer-consumer threads and log\n");
    printf("  ipc                     POSIX shared memory and semaphores\n");
    printf("  banker                  Banker's safe and unsafe tests\n");
    printf("  workers                 normal and simulated-failure children\n");
    printf("  limits                  show actual setrlimit effect\n");
    printf("  cleanup                 demonstrate thread/process/IPC cleanup\n");
    printf("  all        [file]       integrated demonstration and summary\n");
    printf("Default job file : tests/jobs.txt\n");
}

static int get_quantum(const char *input,int *quantum)
{
    if(input==NULL) {*quantum=2;return 1;}
    char *end;
    errno=0;
    long value=strtol(input,&end,10);
    if(errno || *input=='\0' || *end!='\0' || value<1 || value>1000)
    {
        fprintf(stderr,"invalid quantum '%s' : enter an integer from 1 to 1000\n",input);
        return 0;
    }
    *quantum=(int)value;
    return 1;
}

static int compare(const job_t jobs[],int n)
{
    result_t a,b,c;
    if(!run_schedule(jobs,n,"fcfs",2,&a) || !run_schedule(jobs,n,"rr",2,&b) ||
       !run_schedule(jobs,n,"priority",2,&c)) return 0;
    printf("\n=== SAME JOB SET : SCHEDULING COMPARISON ===\n");
    printf("%-15s %-15s %-15s %-10s\n","Algorithm","Avg WT","Avg TAT","Finish");
    printf("%-15s %-15.2f %-15.2f %-10d\n","FCFS",a.avg_wait,a.avg_turn,a.finish_time);
    printf("%-15s %-15.2f %-15.2f %-10d\n","RR (q=2)",b.avg_wait,b.avg_turn,b.finish_time);
    printf("%-15s %-15.2f %-15.2f %-10d\n","Priority",c.avg_wait,c.avg_turn,c.finish_time);
    return 1;
}

int run_cleanup(void)
{
    printf("\n=== CLEANUP VERIFICATION ===\n");
    int threads=run_logger();
    int ipc=run_ipc();
    int workers=run_workers();
    printf("\nCLEANUP SUMMARY : threads %s, IPC %s, workers %s\n",
           threads?"PASS":"FAIL",ipc?"PASS":"FAIL",workers?"PASS":"FAIL");
    return threads && ipc && workers;
}

int main(int argc,char *argv[])
{
    if(argc<2) {show_help(argv[0]);return 1;}
    const char *cmd=argv[1];
    if(strcmp(cmd,"help")==0 && argc==2) {show_help(argv[0]);return 0;}
    if(strcmp(cmd,"logger")==0 && argc==2) return run_logger()?0:1;
    if(strcmp(cmd,"ipc")==0 && argc==2) return run_ipc()?0:1;
    if(strcmp(cmd,"banker")==0 && argc==2) return run_banker()?0:1;
    if(strcmp(cmd,"workers")==0 && argc==2) return run_workers()?0:1;
    if(strcmp(cmd,"limits")==0 && argc==2) return run_limits()?0:1;
    if(strcmp(cmd,"cleanup")==0 && argc==2) return run_cleanup()?0:1;

    int scheduling=strcmp(cmd,"jobs")==0 || strcmp(cmd,"fcfs")==0 ||
                   strcmp(cmd,"rr")==0 || strcmp(cmd,"priority")==0 ||
                   strcmp(cmd,"compare")==0 || strcmp(cmd,"all")==0;
    if(!scheduling)
    {
        fprintf(stderr,"unknown command or incorrect arguments : %s\n",cmd);
        show_help(argv[0]);
        return 1;
    }
    int is_rr=strcmp(cmd,"rr")==0;
    if(argc>(is_rr?4:3))
    {
        fprintf(stderr,"too many arguments for '%s'\n",cmd);
        show_help(argv[0]);
        return 1;
    }
    int quantum=2;
    if(is_rr && argc==4 && !get_quantum(argv[3],&quantum)) return 1;
    const char *file=argc>=3 ? argv[2] : "tests/jobs.txt";
    job_t jobs[MAX_JOBS];
    int n=0;
    if(!read_jobs(file,jobs,&n)) return 1;
    if(strcmp(cmd,"jobs")==0) {show_jobs(jobs,n);return 0;}
    result_t result;
    if(strcmp(cmd,"fcfs")==0 || strcmp(cmd,"priority")==0 || is_rr)
        return run_schedule(jobs,n,cmd,quantum,&result)?0:1;
    if(strcmp(cmd,"compare")==0) return compare(jobs,n)?0:1;

    printf("\n######## CAMPUSCOMPUTE INTEGRATED DEMONSTRATION ########\n");
    int pass=0,total=6;
    show_jobs(jobs,n);
    if(compare(jobs,n)) pass++;
    if(run_logger()) pass++;
    if(run_ipc()) pass++;
    if(run_banker()) pass++;
    if(run_workers()) pass++;
    if(run_limits()) pass++;
    printf("\n=== INTEGRATED SUMMARY ===\n");
    printf("functional module groups passed : %d/%d\n",pass,total);
    printf("integrated demonstration : %s\n",pass==total?"PASS":"FAIL");
    return pass==total?0:1;
}
