/* context_switch.c
 *
 * This program measures the time required for a context switch.
 *
 * Author: Asmit De | U72377278
 * Date: 01/27/2016
 */

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define NANOSECONDS 1000000000
#define WRITE_COUNT 1000

/* Uncomment the line below to turn on logging */
/* 
#define ENABLE_LOG
*/

int main()
{
    pid_t pid;
    struct timespec startTime, endTime;
    unsigned long long endTimeValue = 0, elapsedTime, totalTime = 0;
    int pipefd1[2], pipefd2[2], status, i;
    cpu_set_t mask;
    char buffer[80];
    
    /* Get the affinity mask for the parent process */
    if (sched_getaffinity(getpid(), sizeof(mask), &mask) == -1)
    {
        perror("Affinity Mask error");
        exit(EXIT_FAILURE);
    }
    
    /* Create a pipe to send message from parent process to child process */
    if (pipe(pipefd1) == -1)
    {
        perror("Pipe error");
        exit(EXIT_FAILURE);
    }
    
    /* Create a pipe to send message from child process to parent process */
    if (pipe(pipefd2) == -1)
    {
        perror("Pipe error");
        exit(EXIT_FAILURE);
    }

    /* Spawn a new process */
    pid = fork();

    /* Get the time tick immediately after creating a new process */
    clock_gettime(CLOCK_MONOTONIC_RAW, &endTime);

    switch (pid)
    {
    case -1:
        perror("Fork error");
        exit(EXIT_FAILURE);
        
    case 0:
#ifdef ENABLE_LOG
        printf("\nChild process created with pid: %u", getpid());
#endif        

        /* Set affinity mask for the child process */
        if (sched_setaffinity(getpid(), sizeof(mask), &mask) == -1)
        {
            perror("Affinity Mask error");
            exit(EXIT_FAILURE);
        }
        
        /* Close write end of first pipe and read end of second pipe */
        close(pipefd1[1]);
        close(pipefd2[0]);
        
        /* Read from the first pipe and send acknowledgement to parent*/
        for (i = 0; i <= WRITE_COUNT; i++)
        {
            if (read(pipefd1[0], buffer, sizeof(buffer)) > 0)
            {
                /* Get the time tick just after finishing read */
                clock_gettime(CLOCK_MONOTONIC_RAW, &endTime);

                /* Send the time tick value to parent process through pipe */ 
                memset(buffer, '\0', sizeof(buffer));
                sprintf(buffer, "%llu", (unsigned long long) (NANOSECONDS * 
                    endTime.tv_sec + endTime.tv_nsec));
                write(pipefd2[1], buffer, sizeof(buffer));
                memset(buffer, '\0', sizeof(buffer));
            }
        }

        /* Close read end of first pipe and write end of the second pipe */
        close(pipefd1[0]);
        close(pipefd2[1]);

        /* Close the child process */
        _exit(EXIT_SUCCESS);

    default:
        /* Close read end of first pipe and write end of second pipe */
        close(pipefd1[0]);
        close(pipefd2[1]);

        /* Write to the first pipe and wait for acknowledgement from child */
        for (i = 0; i <= WRITE_COUNT; i++)
        {
            memset(buffer, '\0', sizeof(buffer));
            sprintf(buffer, "%d", i);
            if (write(pipefd1[1], buffer, sizeof(buffer)) > 0)
            {
                /* Get the time tick just before waiting to read */
                clock_gettime(CLOCK_MONOTONIC_RAW, &startTime);

                memset(buffer, '\0', sizeof(buffer));
                if (read(pipefd2[0], buffer, sizeof(buffer)) > 0)
                {
                    /* We skip the 0th iteration as it takes into account the
                     * time taken to execute the initial code for the child
                     * including the sched_setaffinity() call and hence gives
                     * a very inaccurate and high value
                     */
                    if (i == 0) continue;
                    
                    /* Obtain the end time tick value returned by the child */
                    sscanf(buffer, "%llu", &endTimeValue);

                    /* Calculate the time taken for context switching */
                    elapsedTime = endTimeValue - (NANOSECONDS * startTime.tv_sec
                                    + startTime.tv_nsec);

#ifdef ENABLE_LOG                
                    printf("\n%llu", elapsedTime);
#endif

                    totalTime += elapsedTime;
                }
            }
        }

        /* Close write end of first pipe and read end of second pipe */
        close(pipefd1[1]);
        close(pipefd2[0]);
    }

    /* Wait for the child process to finish */
    if((pid = wait(&status)))
    {
#ifdef ENABLE_LOG
        printf("\nChild process %u terminated successfully", pid);
#endif
    }

    /* After termination of the child process, calculate the average time taken
     * for context switching
     */
    printf("\nTotal write count: %d", WRITE_COUNT);
    printf("\nAverage context switching time: %f ns\n", (float) totalTime / 
        WRITE_COUNT);

    return 0;
}

