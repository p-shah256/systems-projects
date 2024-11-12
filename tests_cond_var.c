/*
 * file:        test.c
 * description: basic unit tests for Lab 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "qthread.h"
#include <sys/time.h>

qthread_mutex_t* mutex;
qthread_cond_t* cond;

/*****************************************/
/* // Test 1: Signal wakes one at a time */
/*****************************************/
void* wait_thread(void* arg) {
    int id = *(int*)arg;
    printf("thread %d locking mutex and going to sleep on cond var\n", id);
    qthread_mutex_lock(mutex);
    qthread_cond_wait(cond, mutex);
    printf("thread %d woken up and unlocking mutex\n", id);
    qthread_mutex_unlock(mutex);
    return arg;
}

void cond_wait(void) {
    printf("\n=== Testing cond_wait ===\n");
    mutex = qthread_mutex_create();
    cond = qthread_cond_create();
    int ids[3] = {1, 2, 3};
    qthread_t t[3];
    for(int i = 0; i < 3; i++) {
        t[i] = qthread_create(wait_thread, &ids[i]); // create 3 threads
        // each thread increments flag and waits
    }
	printf("main thread yeilding to make all wait...\n");
    qthread_yield();  // Let threads wait

    for(int i = 0; i < 3; i++) {
        qthread_cond_signal(cond); // wake and let them finish
		printf("thread %d exited... yeilding\n", i);
        qthread_yield(); // main thread yields to let others run
    }
    for(int i = 0; i < 3; i++) {
        void* val = qthread_join(t[i]); // join
        assert(*(int*)val == i + 1);  // Assert the returned value matches the thread ID
    }
    qthread_mutex_destroy(mutex);
    qthread_cond_destroy(cond);
}

/**********************************/
/* // Test 2: Broadcast wakes all */
/**********************************/
void cond_broadcast(void) {
    printf("\n=== Testing cond_broadcast ===\n");
    mutex = qthread_mutex_create();
    cond = qthread_cond_create();

    int ids[3] = {1, 2, 3};
    qthread_t t[3];
    for(int i = 0; i < 3; i++) {
        t[i] = qthread_create(wait_thread, &ids[i]);
    }

    qthread_yield();  // Let threads wait
    qthread_cond_broadcast(cond);

    for(int i = 0; i < 3; i++) {
        qthread_join(t[i]);
    }

    qthread_mutex_destroy(mutex);
    qthread_cond_destroy(cond);
}


/***************************************/
/* // Test 3: Wait twice, signal twice */
/***************************************/
void* double_wait_thread(void* arg) {
    printf("[Thread] Starting double wait thread\n");

    printf("[Thread] Attempting to lock mutex\n");
    qthread_mutex_lock(mutex);
    printf("[Thread] Mutex locked successfully\n");

    printf("[Thread] Entering first wait state\n");
    qthread_cond_wait(cond, mutex);
    printf("[Thread] Woken up from first wait\n");

    printf("[Thread] Entering second wait state\n");
    qthread_cond_wait(cond, mutex);
    printf("[Thread] Woken up from second wait\n");

    printf("[Thread] Unlocking mutex\n");
    qthread_mutex_unlock(mutex);
    printf("[Thread] Mutex unlocked, thread finishing\n");

    return NULL;
}

void cond_wait2(void) {
    printf("\n=== Testing cond_wait2 ===\n");

    printf("[Main] Creating mutex and condition variable\n");
    mutex = qthread_mutex_create();
    cond = qthread_cond_create();

    printf("[Main] Creating worker thread\n");
    qthread_t t = qthread_create(double_wait_thread, NULL);

    printf("[Main] Yielding to let worker thread start\n");
    qthread_yield();

    printf("[Main] Sending first signal\n");
    qthread_cond_signal(cond);

    printf("[Main] Yielding to let worker process first signal\n");
    qthread_yield();

    printf("[Main] Sending second signal\n");
    qthread_cond_signal(cond);

    printf("[Main] Waiting for worker thread to complete\n");
    qthread_join(t);
    printf("[Main] Worker thread joined successfully\n");

    printf("[Main] Cleaning up - destroying mutex and condition variable\n");
    qthread_mutex_destroy(mutex);
    qthread_cond_destroy(cond);

    printf("=== Test cond_wait2 completed ===\n");
}
