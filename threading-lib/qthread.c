/*
 * file:        qthread.c
 * description: assignment - simple emulation of POSIX threads
 * class:       CS 5600, Fall 2019
 */

/* a bunch of includes which will be useful */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "qthread.h"
#include "qthread_manager.c"

//Working with up to 4 threads, will be at most using 3
#define NUM_THREADS 4
#define STACK_SIZE 64*1024
/* prototypes for stack.c and switch.s
 * see source files for additional details
 */
extern void switch_thread(void **location_for_old_sp, void *new_value);
static long get_usecs(void);

/* Mutex and cond structures - @allocate them in qthread_mutex_create /
 * qthread_cond_create and free them in @the corresponding _destroy functions.
 */
struct qthread_mutex {
    uint16_t locked;
    struct threadq *queue;
};

struct qthread_cond
{
    struct threadq *queue;
};
typedef struct qthread_mutex qthread_mutex_t;
typedef struct qthread_cond qthread_cond_t;
/* Mutex functions
 */
qthread_mutex_t *qthread_mutex_create(void)
{
  struct qthread_mutex *mutex = malloc(sizeof(struct qthread_mutex));
  mutex->locked = 0;
  mutex->queue = malloc(sizeof(struct threadq));
  mutex->queue->size = 0;
  return mutex;
}

void qthread_mutex_destroy(qthread_mutex_t *mutex)
{
  free(mutex);
  return;
}

void qthread_mutex_lock(qthread_mutex_t *mutex)
{
  if(mutex->locked == 0){
    mutex->locked = 1;
    return;
  }
  else{
    push_back(mutex->queue,current_thread);
    schedule(2);
  }
}
void qthread_mutex_unlock(qthread_mutex_t *mutex)
{
  if(mutex->queue->size > 0){
    qthread_t tmp = pop_front(mutex->queue);
    if(tmp != NULL){
        push_back(runnable_queue,tmp);
    }
	return;
  }
  else{
    mutex->locked = 0;
    return;
  }
}

/* Condition variable functions
 */
qthread_cond_t *qthread_cond_create(void)
{
    qthread_cond_t *cond = malloc(sizeof(struct qthread_cond));
    cond->queue = malloc(sizeof(struct threadq));
    cond->queue->size = 0;
    return cond;
}
void qthread_cond_destroy(qthread_cond_t *cond)
{
  free(cond);
  return;
}

void qthread_cond_wait(qthread_cond_t *cond, qthread_mutex_t *mutex)
{
    qthread_mutex_unlock(mutex);
    push_back(cond->queue,current_thread);
    schedule(2);
    qthread_mutex_lock(mutex);
}


void qthread_cond_signal(qthread_cond_t *cond)
{
    if(cond->queue != NULL && cond->queue->size > 0){
      qthread_t tmp = pop_front(cond->queue);
      push_back(runnable_queue,tmp);
    }
}

void qthread_cond_broadcast(qthread_cond_t *cond)
{
    while(cond->queue->size > 0){
      if(cond->queue != NULL && cond->queue->size > 0){
        qthread_t tmp = pop_front(cond->queue);
        push_back(runnable_queue,tmp);
      }
    }
}


/* Helper function for POSIX replacement API - you'll need to tell
 * time in order to implement qthread_usleep.
 * WARNING - store return value in 'long' (64 bits), not 'int' (32 bits)
 */
static long get_usecs(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000000 + tv.tv_usec;
}
