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
#include <assert.h>
#include <sys/time.h>
#include <errno.h>
#include "qthread.h"
#include "qthread_manager.c"

//Working with up to 4 threads, will be at most using 3
#define NUM_THREADS 4
#define STACK_SIZE 64*1024
/* prototypes for stack.c and switch.s
 * see source files for additional details
 */
extern void switch_thread(void **location_for_old_sp, void *new_value);
// HINT: pushes a fake stack to return to func(arg1, arg2);
//       MOSTLY should push a wrapper function that exits after the thread is done
//       so that whenever we switch we execute the func
//       F is the wrapper
//       f2 is the function that should be called with arg =
//       wrapper calls f2(arg)
//       returns a stack pointer
extern void *setup_stack(void *_stack, size_t len, f_2arg_t f, f_1arg_t f2, void *arg);

/* Mutex and cond structures - @allocate them in qthread_mutex_create /
 * qthread_cond_create and free them in @the corresponding _destroy functions.
 */
struct qthread_mutex {
    /* mutex holds flag and queue, flag is whether mutex is locked or not. */;
    uint16_t flag;
    struct threadq *queue;
};

struct qthread_cond
{
    /* conditional variables is a queue of thread structures */;
    struct threadq *queue;
};

/* I suggest factoring your code so that you have a 'schedule'
 * function which selects the next thread to run and @switches to it,
 * or goes to sleep if there aren't any threads left to run.
 *
 * NOTE - if you end up switching back to the same thread, do *NOT*
 * use do_switch - check for this case and return from schedule(),
 * or else @you'll crash.
 */
void schedule();



/* qthread_yield - yield to the next @runnable thread.
 */
/*void qthread_yield(void)
{
    /* your code here */
//}

/* qthread_exit, qthread_join - exit argument is returned by
 * qthread_join. Note that join blocks if the thread hasn't exited
 * yet, and is allowed to crash @if the thread doesn't exist.
 */
/*void qthread_exit(void *val)
{
    /* your code here */
//}
//void *qthread_join(qthread_t thread)
//{
    /* your code here */
//}

/* Mutex functions
 */
qthread_mutex_t *qthread_mutex_create(void)
{
  struct qthread_mutex *mutex = malloc(sizeof(struct qthread_mutex));
  return mutex;
}

void qthread_mutex_destroy(qthread_mutex_t *mutex)
{
  free(mutex);
  return;
}

void qthread_mutex_lock(qthread_mutex_t *mutex)
{
  if(!mutex->flag == 1){
    mutex->flag = 1;
    return;
  }
  else{
    push_back(mutex->queue,&current);
    schedule();
  }
}
void qthread_mutex_unlock(qthread_mutex_t *mutex)
{
  if(isEmpty(mutex->queue) == 1){
    struct qthread tmp = pop_front(mutex->queue);
    push_back(&active,&tmp);
  }
  else{
    return;
  }
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
