/* DO NOT remove includes */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>
#include "surfers.h"
#include "surfers_test.c"

/* Declare synchronization variables */
int _readyToSurf, _surfersInWater, _readyToLeave;
pthread_mutex_t readyToSurfLock, surfersInWaterLock, readyToLeaveLock;
pthread_cond_t canSurf, canLeave;


/* Add code to surfer's thread. Surfer MUST call getReady, surf, and leave (in that order) */
void surfer(void *dptr) {
    dataT *d=(dataT *)dptr;
    struct sigaction sa;
    
    /* Define signal handler for surfer thread */
    void sigusr2_handler(int sig)
    {
        pthread_mutex_lock(&readyToSurfLock);
        DPRINTF("\ns%d received signal to leave", d->id);
        leave(d);
        pthread_mutex_unlock(&readyToSurfLock);
        
        pthread_exit(NULL);
    }
    
    /* Register signal handler for main thread */
    sa.sa_handler = sigusr2_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR2, &sa, NULL);
    
    
    /* Surfer is arrives and gets ready */
    pthread_mutex_lock(&readyToSurfLock);
        //if(d->id > 1) //THIS HAS CHANGED
    getReady(d);
    _readyToSurf++;
    DPRINTF("\ns%d is ready, _readyToSurf = %d", d->id, _readyToSurf);
    /* Signal any surfer if waiting to surf */
    pthread_cond_signal(&canSurf);
    pthread_mutex_unlock(&readyToSurfLock);

    
    /*******************************
     * Surfer is ready to surf now *
     *******************************/
    
    
    /* If alone, wait for another surfer to get ready before going in the water */
    pthread_mutex_lock(&readyToSurfLock);
    while (_readyToSurf < 2)
    {
        /*  Check the number of surfers in water. If greater than 0,
         *  he can go into the water. If 0, he waits for another surfer
         *  to get ready to surf.
         */
        pthread_mutex_lock(&surfersInWaterLock);
        if (_surfersInWater != 0)
        {
            pthread_mutex_unlock(&surfersInWaterLock);
            break;
        }
        else
        {
            pthread_mutex_unlock(&surfersInWaterLock);
        }
        
        DPRINTF("\ns%d is waiting for the other surfers to arrive", d->id);
        
        pthread_cond_wait(&canSurf, &readyToSurfLock);
        
        DPRINTF("\ns%d received ready to surf signal from other surfer to surf", d->id);
    }
    pthread_mutex_unlock(&readyToSurfLock);
    
    DPRINTF("\ns%d can now surf", d->id);
    
    /* Initiate surfing */
    pthread_mutex_lock(&readyToSurfLock);
    pthread_mutex_lock(&surfersInWaterLock);
    surf(d);
    _surfersInWater++;
    DPRINTF(", _surfersInWater = %d", _surfersInWater);
    /* Signal any surfer waiting to leave, as that surfer can leave
     * since there is one more surfer in water now.
     */
    pthread_cond_signal(&canLeave);
    pthread_mutex_unlock(&surfersInWaterLock);
    _readyToSurf--;
    DPRINTF(", _readyToSurf = %d", _readyToSurf);
    pthread_mutex_unlock(&readyToSurfLock);
    
    /* Ensure that the other surfer comes in with him,
     * so that he is not alone in water.
     */
    while (_surfersInWater == 1);
    
    
    /*************************
     * Surfer is surfing now *
     *************************/
    
    
    /* Signal any waiting surfer ready to leave */
    pthread_mutex_lock(&readyToLeaveLock);
    _readyToLeave++;
    DPRINTF("\ns%d is ready to leave, %d", d->id, _readyToLeave);
    pthread_cond_signal(&canLeave);
    pthread_mutex_unlock(&readyToLeaveLock);

    /* Before leaving, ensure he does not leave any surfer alone in water */
    pthread_mutex_lock(&readyToLeaveLock);
    while (_readyToLeave < 2)
    {
        /*  Check the number of surfers in water (including himself).
        *  If greater than 2, he can leave. If less than 2, that
        *  means he is leaving alongside the penultimate surfer, to whom
        *  he just signalled. If equal to 2, he waits for the
        *  other surfer's signal to ready to leave. 
        */  
        pthread_mutex_lock(&surfersInWaterLock);
        if (_surfersInWater != 2)
        {
            pthread_mutex_unlock(&surfersInWaterLock);
            break;
        }
        pthread_mutex_unlock(&surfersInWaterLock);
        
        DPRINTF("\ns%d is waiting for the other surfer in water to finish surfing", d->id);
        
        pthread_cond_wait(&canLeave, &readyToLeaveLock);
        
        DPRINTF("\ns%d received ready to leave signal from other surfer to leave", d->id);
    }
    pthread_mutex_unlock(&readyToLeaveLock);
    
    DPRINTF("\ns%d can now leave", d->id);
    
    /* Initiate leave */
    pthread_mutex_lock(&readyToLeaveLock);
    pthread_mutex_lock(&surfersInWaterLock);
    leave(d);
    _surfersInWater--;
    DPRINTF(", _surfersInWater = %d", _surfersInWater);
    pthread_mutex_unlock(&surfersInWaterLock);
    _readyToLeave--;
    DPRINTF(", _readyToLeave = %d", _readyToLeave);
    pthread_mutex_unlock(&readyToLeaveLock);
    
    
    /***********************
     * Surfer has left now *
     ***********************/
}

/* Add code to main (DO NOT remove initialization code) */
int main() {
    int j=0, rc;
    struct sigaction sa;

    /* Initialize synchronization variables */
    if (sem_init(&dusk, 0, 0) == -1) { perror("sem_init"); } // THIS HAS CHANGED
    _readyToSurf = 0;
    pthread_mutex_init(&readyToSurfLock, NULL);
    _surfersInWater = 0;
    pthread_mutex_init(&surfersInWaterLock, NULL);
    _readyToLeave = 0;
    pthread_mutex_init(&readyToLeaveLock, NULL);
    pthread_cond_init(&canSurf, NULL);
    pthread_cond_init(&canLeave, NULL);
    sem_init(&monitor_mutex, 0, 1);
    

    /* Initialize thread data structures */
    pthread_t t[NSURFERS];
    dataT **ds = malloc(sizeof(dataT) * NSURFERS);
    for (j=0; j<NSURFERS; j++) { ds[j] = malloc(sizeof(struct data)); }

    /* s1 and s2 start surfing */ //THIS HAS CHANGED
    /*surf(ds[0]);  //THIS HAS CHANGED
        surf(ds[1]);*/ //THIS HAS CHANGED
        
    /* Define the signal handler for main thread */
    void sigusr1_handler(int sig)
    {
        int i;
        
        DPRINTF("\nReceived dusk signal from monitor");
        
        /*  Check if any of the surfers are in READY or SURFING state,
         *  and signal them to leave
         */
        for (i = 0; i < NSURFERS; i++)
        {
            sem_wait(&monitor_mutex);
            if (ds[i]->state == READY || ds[i]->state == SURFING)
            {
                sem_post(&monitor_mutex);
                pthread_kill(t[i], SIGUSR2);
            }
            else
            {
                sem_post(&monitor_mutex);
            }
        }
    }

    /* Create monitor */
    pthread_t mon;
    pthread_create(&mon, NULL, (void *)&monitor, (void *)ds);
    printf("The sharks are in the water");

    /* Register signal handler for main thread */
    sa.sa_handler = sigusr1_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    
    /* Create surfers */
    for (j = 0; j < NSURFERS; j++)
    {
        ds[j]->id = j;
        rc = pthread_create(&t[j], NULL, (void *) &surfer, (void *) ds[j]);
        assert(rc == 0);
    }
    
    /* Wait for surfers to finish */
    for (j = 0; j < NSURFERS; j++)
    {
        rc = pthread_join(t[j], NULL);
        assert(rc == 0);
    }

    /* Wait for monitor to finish */
    pthread_join(mon, NULL);

    /* Clean up synchronization variables */ 
    pthread_mutex_destroy(&readyToSurfLock);
    pthread_mutex_destroy(&readyToLeaveLock);
    pthread_mutex_destroy(&surfersInWaterLock);
    pthread_cond_destroy(&canSurf);
    pthread_cond_destroy(&canLeave);
    sem_destroy(&dusk);
    sem_destroy(&monitor_mutex);

    return 0;
}


