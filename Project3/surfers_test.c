//#include "surfers.h" /* has dataT, NSURFERS */ // ->> Commented this out as it is already included in surfers.c

/* Set the time till dusk in seconds */
#define TILL_DUSK 1

/* Define a debug printf() fuction that can be activated on comiling with -D DEBUG */
#ifdef DEBUG
#define DPRINTF printf
#else
#define DPRINTF(format, args...)
#endif

void surf(dataT *d) {
    
    sem_wait(&monitor_mutex);
    d->state = SURFING;
    printf("\ns%d is surfing", d->id);
    sem_post(&monitor_mutex);
}

void leave(dataT *d) {
    
    sem_wait(&monitor_mutex);
    d->state = LEAVE;
    printf("\ns%d leaves", d->id);
    sem_post(&monitor_mutex);
}

void getReady(dataT *d) {
    
    sem_wait(&monitor_mutex);
    d->state = READY;
    printf("\ns%d arrives", d->id);
    sem_post(&monitor_mutex);
}

void monitor(void * x) {
    
    struct timespec ts;
    
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += TILL_DUSK;
    
    if(sem_timedwait(&dusk, &ts) == -1 && errno == ETIMEDOUT)
    {
        DPRINTF("\nIt's dusk. Signalling surfers, if any, to leave.\n");
        /*  We dont have accesss to the thread ids for surfers in this method,
         *  so we send a signal to the main thread (who has the surfer thread ids).
         *  The main thread will again signal each of the active (READY or SURFING)
         *  threads to LEAVE.
         * 
         *  If we had access to the thread ids, we wouldn't have to do this is such
         *  a roundabout way. We could've sent the signal directly to the surfer threads.
         *  -_-+
         */
        kill(getpid(), SIGUSR1);
    }
}
