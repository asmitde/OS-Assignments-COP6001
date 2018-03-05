/* DO NOT CHANGE THIS FILE */
#include <semaphore.h>

#ifndef NSURFERS
#define NSURFERS 15
#endif

#define READY 0
#define SURFING 1
#define LEAVE 2

sem_t monitor_mutex;
sem_t dusk; // THIS HAS CHANGED

typedef struct data {
  int id;
  int state;
} dataT;

void getReady(dataT * d);
void surf(dataT * d);
void leave(dataT * d);

void surfer(void *dptr);

void monitor(void * dsptr);

