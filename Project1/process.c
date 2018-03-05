/* process.c
 *
 * This program measures the time required to spawn a new process.
 *
 * Author: Asmit De | U72377278
 * Date: 01/20/2016
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define NANOSECONDS 1000000000
#define NUM_PROCESSES 1000

/* Uncomment the line below to turn on logging */
/*
#define ENABLE_LOG
*/

int main()
{
    pid_t pid;
    struct timespec startTime, endTime;
    unsigned long elapsedTime, totalTime = 0;
    int status, processCounter = 0, i;

    for (i = 0; i < NUM_PROCESSES; i++)
    {
        /* Get the time tick just before creating a new process */
        clock_gettime(CLOCK_MONOTONIC_RAW, &startTime);

        /* Spawn a new process */
        pid = fork();

        /* Get the time tick immediately after creating a new process */
        clock_gettime(CLOCK_MONOTONIC_RAW, &endTime);

        /* Log the child process creation and terminate the process */
        if (pid == 0)
        {
#ifdef ENABLE_LOG
            printf("\nChild process created with pid: %u", getpid());
#endif
            _exit(EXIT_SUCCESS);
        }
        else if (pid == -1)
        {
            perror("Fork error");
            continue;
        }
        else
        {
            /* Calculate the time elapsed to fork the child process */
            elapsedTime = NANOSECONDS * (endTime.tv_sec - startTime.tv_sec) + 
                            (endTime.tv_nsec - startTime.tv_nsec);

#ifdef ENABLE_LOG
            printf("\n%lu", elapsedTime);
#endif

            totalTime += elapsedTime;
            processCounter++;
        }
    }

    /* Wait for the child processes to finish */
    for (i = 0; i < processCounter; i++)
    {
        if((pid = wait(&status)))
        {
#ifdef ENABLE_LOG
            printf("\nChild process %u terminated successfully", pid);
#endif
        }
    }

    /* After termination of the child processes, calculate the average process 
     * creation time
     */
    printf("\nTotal child processes created: %d", processCounter);
    printf("\nAverage creation time for each process: %f ns\n",
       (float) totalTime / processCounter);

    return 0;
}

