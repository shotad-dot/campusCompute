#include <stdio.h>
#include "campus.h"

#define P 5
#define R 3

static int check_safe(const int available[R],const int allocation[P][R],const int maximum[P][R])
{
    int need[P][R],work[R],finished[P]={0},sequence[P],count=0;
    for(int j=0;j<R;j++) work[j]=available[j];
    printf("Available : ");
    for(int j=0;j<R;j++) printf("%d ",available[j]);
    printf("\nProcess    Allocation    Maximum       Need\n");
    for(int i=0;i<P;i++)
    {
        printf("P%-10d",i);
        for(int j=0;j<R;j++) printf("%d ",allocation[i][j]);
        printf("        ");
        for(int j=0;j<R;j++) printf("%d ",maximum[i][j]);
        printf("        ");
        for(int j=0;j<R;j++)
        {
            need[i][j]=maximum[i][j]-allocation[i][j];
            printf("%d ",need[i][j]);
        }
        printf("\n");
    }
    printf("Need = Maximum - Allocation\n");
    while(count<P)
    {
        int found=0;
        for(int i=0;i<P;i++)
        {
            if(finished[i]) continue;
            int possible=1;
            for(int j=0;j<R;j++) if(need[i][j]>work[j]) possible=0;
            if(possible)
            {
                // A finished process returns its allocation, so that much becomes available again.
                for(int j=0;j<R;j++) work[j]+=allocation[i][j];
                finished[i]=1;
                sequence[count++]=i;
                found=1;
            }
        }
        if(!found) break;
    }
    if(count==P)
    {
        printf("SAFE state; safe sequence : ");
        for(int i=0;i<P;i++) printf("P%d%s",sequence[i],i==P-1?"\n":" -> ");
        return 1;
    }
    printf("UNSAFE state; only %d of %d processes could finish, no complete safe sequence\n",count,P);
    return 0;
}

int run_banker(void)
{
    const int alloc[P][R]={{0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2}};
    const int max[P][R]={{7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}};
    const int safe[R]={3,3,2},unsafe[R]={0,0,0};
    printf("\n=== BANKER'S ALGORITHM : SAFE TEST ===\n");
    int a=check_safe(safe,alloc,max);
    printf("\n=== BANKER'S ALGORITHM : UNSAFE TEST ===\n");
    int b=check_safe(unsafe,alloc,max);
    return a && !b;
}
