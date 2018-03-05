/* thread.c
 *
 * This program measures the time required to create a new thread.
 *
 * Author: Asmit De | U72377278
 * Date: 01/21/2016
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NANOSECONDS 1000000000
#define NUM_THREADS 1000

/* Uncomment the line below to turn on logging */
/*
#define ENABLE_LOG
*/

void *start_routine(void *arg);

int main()
{
    pthread_t threads[NUM_THREADS];
    struct timespec startTime, endTime;
    unsigned long elapsedTime, totalTime = 0;
    int threadCounter = 0, retVal, i;

    for (i = 0; i < NUM_THREADS; i++)
    {
        /* Get the time tick just before creating a new thread */
        clock_gettime(CLOCK_MONOTONIC_RAW, &startTime);

        /* Create a new thread */
        retVal = pthread_create(&threads[i], NULL, start_routine, NULL);

        /* Get the time tick immediately after creating a new thread */
        clock_gettime(CLOCK_MONOTONIC_RAW, &endTime);

        /* Handle thread creation errors */
        if (retVal != 0)
        {
            errno = retVal;
            perror("Thread creation error");
            continue;
        }

        /* Calculate the time elapsed to create the thread */
        elapsedTime = NANOSECONDS * (endTime.tv_sec - startTime.tv_sec) + 
                        (endTime.tv_nsec - startTime.tv_nsec);

#ifdef ENABLE_LOG
        printf("\n%lu", elapsedTime);
#endif

        totalTime += elapsedTime;
        threadCounter++;
    }

    /* Wait for all threads to finish */
    for (i = 0; i < NUM_THREADS; i++)
    {
        /* Skip threads which had not been created */
        if (!threads[i]) continue;
        
        if ((retVal = pthread_join(threads[i], NULL)) == 0)
        {
#ifdef ENABLE_LOG
            printf("\nThread %u terminated successfully", 
                (unsigned int) threads[i]);
#endif
        }
        else
        {
            errno = retVal;
            perror("Thread join error");
        }
    }

    /* After termination of the threads, calculate the average thread creation
     * time
     */
    printf("\nTotal threads created: %d", threadCounter);
    printf("\nAverage creation time for each thread: %f ns\n", 
        (float) totalTime / threadCounter);

    return 0;
}

/* Thread routine */
void *start_routine(void *arg)
{
#ifdef ENABLE_LOG
    printf("\nThread created with tid: %u", (unsigned int) pthread_self());
#endif

    pthread_exit(EXIT_SUCCESS);
}

