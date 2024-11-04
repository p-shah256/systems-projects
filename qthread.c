/*
 * file:        qthread.c
 * description: assignment - simple emulation of POSIX threads
 * class:       CS 5600, Fall 2019
 */

/* a bunch of includes which will be useful */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <errno.h>
#include "qthread.h"

//Working with up to 4 threads, will be at most using 3
#define NUM_THREADS 4
/* prototypes for stack.c and switch.s
 * see source files for additional details
 */
extern void switch_thread(void **location_for_old_sp, void *new_value);
extern void start_thread(void *stack, void *func, void *arg1, void *arg2);
extern void *setup_stack(void *_stack, size_t len, void *func, void *arg1, void *arg2);

/* this is your qthread structure.
 */
struct qthread {
    struct qthread* next;
    void* saved_sp;
    void* stack;
};

/* You'll probably want to define a thread queue structure, and
 * functions to append and remove threads. (Note that you only need to
 * remove the oldest item from the head, makes removal a lot easier)
 */

//structure is holding the array of threads and allows for adding or taking away from queue.
struct threadq {
    struct qthread* front;
    struct qthread* back;
};

int isEmpty(struct threadq* q) {
  if(q->front == q->back && q->back == NULL) {
    return 1;
  }
    return 0;
}

void enqueue(struct threadq *queue,struct qthread *thread) {
 if (queue->back == NULL) {
   queue->front = queue->back = thread;
 }
 else{
  queue->back->next = thread;
 }
 printf("Enqueued thread %p\n",thread);
}

struct qthread* dequeue(struct threadq *queue) {
 if(queue->front == NULL){
   printf("Empty queue to dequeue\n");
   return NULL;
 }
  struct qthread* thread = queue->front;
  queue->front = queue->front->next;
  printf("Dequeued thread %p\n",thread);
  return thread;
}
/* Mutex and cond structures - @allocate them in qthread_mutex_create /
 * qthread_cond_create and free them in @the corresponding _destroy functions.
 */
struct qthread_mutex {
    /* mutex holds flag and queue, flag is whether mutex is locked or not. */;
    uint16_t flag;
    struct threadq *queue;
};

struct qthread_cond {
    /* conditional variables is a queue of thread structures */;
    struct threadq *queue;
};


/* qthread_create - see hints @for how to implement it, especially the
 * reference to a "wrapper" function
 */
qthread_t qthread_create(f_1arg_t f, void *arg1)
{
    /* your code here */
}

/* I suggest factoring your code so that you have a 'schedule'
 * function which selects the next thread to run and @switches to it,
 * or goes to sleep if there aren't any threads left to run.
 *
 * NOTE - if you end up switching back to the same thread, do *NOT*
 * use do_switch - check for this case and return from schedule(),
 * or else @you'll crash.
 */
void schedule(void *save_location);

/* qthread_init - set up a thread structure for the main (OS-provided) thread
 */
void qthread_init(void)
{
    /* your code here */
}

/* qthread_yield - yield to the next @runnable thread.
 */
void qthread_yield(void)
{

}

/* qthread_exit, qthread_join - exit argument is returned by
 * qthread_join. Note that join blocks if the thread hasn't exited
 * yet, and is allowed to crash @if the thread doesn't exist.
 */
void qthread_exit(void *val)
{
    /* your code here */
}
void *qthread_join(qthread_t thread)
{
    /* your code here */
}

/* Mutex functions
 */
qthread_mutex_t *qthread_mutex_create(void)
{
}
void qthread_mutex_destroy(qthread_mutex_t *mutex)
{
}
void qthread_mutex_lock(qthread_mutex_t *mutex)
{
}
void qthread_mutex_unlock(qthread_mutex_t *mutex)
{
}

/* Condition variable functions
 */
qthread_cond_t *qthread_cond_create(void)
{
}
void qthread_cond_destroy(qthread_cond_t *cond)
{
}
void qthread_cond_wait(qthread_cond_t *cond, qthread_mutex_t *mutex)
{
}
void qthread_cond_signal(qthread_cond_t *cond)
{
}
void qthread_cond_broadcast(qthread_cond_t *cond)
{
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

/* POSIX replacement API. This semester we're only implementing 'usleep'
 *
 * If there are no runnable threads, your scheduler needs to wait, 
 * using one or more calls to the system usleep() function, until 
 * a thread blocked in 'qthread_usleep' is ready to wake up. 
 */


/* qthread_usleep - yield to next runnable thread, making arrangements
 * to be put back on the active list after 'usecs' timeout. 
 */
void qthread_usleep(long int usecs)
{
}
