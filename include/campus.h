#ifndef CAMPUS_H
#define CAMPUS_H

#define MAX_JOBS 32

typedef struct
{
    int job_id;
    int arrival_time;
    int burst_time;
    int priority;
    int status;
    int start_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
} job_t;

typedef struct
{
    double avg_wait;
    double avg_turn;
    int finish_time;
} result_t;

int read_jobs(const char *file_name, job_t jobs[], int *n);
void show_jobs(const job_t jobs[], int n);
int run_schedule(const job_t jobs[], int n, const char *mode, int quantum, result_t *result);
int run_logger(void);
int run_ipc(void);
int run_banker(void);
int run_workers(void);
int run_limits(void);
int run_cleanup(void);
void show_help(const char *program);

#endif
