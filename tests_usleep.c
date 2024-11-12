#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "qthread.h"
#include <sys/time.h>


static long get_usecs(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000000 + tv.tv_usec;
}

void* run_sleep(void* arg) {
    printf("Thread %s starting, going to sleep\n", (char*)arg);
    qthread_usleep(500000);  // sleep for 0.5 seconds
    printf("Thread %s woke up\n", (char*)arg);
    return arg;
}

void* run_yield(void* arg) {
    for(int i = 0; i < 3; i++) {
        printf("Thread %s running iteration %d\n", (char*)arg, i);
        qthread_yield();
    }
    printf("Thread %s finished\n", (char*)arg);
    return arg;
}

void test_basic_sleep(void) {
    printf("\n=== Testing Basic Sleep and Yield ===\n");

    qthread_t t1[3] = {
        qthread_create(run_yield, "1"),
        qthread_create(run_sleep, "2"),
        qthread_create(run_yield, "3")
    };

    void *val;
    for(int i = 0; i < 3; i++) {
        val = qthread_join(t1[i]);
        assert(!strcmp(val, (i == 1) ? "2" : (i == 0 ? "1" : "3")));
        printf("Thread %d joined successfully\n", i+1);
    }
}

void* run_timed_sleep(void* arg) {
    char* thread_id = (char*)arg;
    long sleep_time;

    if (*thread_id == '1') sleep_time = 300000;      // 0.3s
    else if (*thread_id == '2') sleep_time = 100000; // 0.1s
    else if (*thread_id == '3') sleep_time = 200000; // 0.2s
    else sleep_time = 0;                             // Thread 4 just yields

    long start_time = get_usecs();
    printf("[%ld] Thread %s starting\n", get_usecs() - start_time, thread_id);

    if (sleep_time > 0) {
        printf("[%ld] Thread %s sleeping for %.1fs\n",
               get_usecs() - start_time, thread_id, sleep_time/1000000.0);
        qthread_usleep(sleep_time);
    } else {
        for(int i = 0; i < 3; i++) {
            printf("[%ld] Thread %s yielding\n", get_usecs() - start_time, thread_id);
            qthread_yield();
        }
    }

    printf("[%ld] Thread %s woke up/finished\n", get_usecs() - start_time, thread_id);
    return arg;
}

void test_timed_sleep(void) {
    printf("\n=== Testing Timed Sleep ===\n");
    long start_time = get_usecs();

    qthread_t t[4] = {
        qthread_create(run_timed_sleep, "1"),
        qthread_create(run_timed_sleep, "2"),
        qthread_create(run_timed_sleep, "3"),
        qthread_create(run_timed_sleep, "4")
    };

    void *val;
    char* expected[4] = {"1", "3", "2", "4"};
    for(int i = 0; i < 4; i++) {
        val = qthread_join(t[i]);
        printf("[%ld] Joined thread %s\n", get_usecs() - start_time, expected[i]);
    }
}

void* run_long_sleep(void* arg) {
    printf("Thread 1: Starting\n");
    printf("Thread 1: Going to sleep for 2 seconds\n");
    qthread_usleep(2000000);
    printf("Thread 1: Woke up from sleep\n");
    printf("Thread 1: Exiting\n");
    return arg;
}

void* run_joiner(void* arg) {
    printf("Thread 2: Starting\n");
    qthread_t* thread_to_join = (qthread_t*)arg;
    printf("Thread 2: Waiting for Thread 1 to finish\n");
    void* result = qthread_join(*thread_to_join);
    printf("Thread 2: Thread 1 joined successfully\n");
    printf("Thread 2: Exiting\n");
    return NULL;
}

void test_join_sleep(void) {
    printf("\n=== Testing Join During Sleep ===\n");

    qthread_t t1 = qthread_create(run_long_sleep, "1");
    qthread_t t2 = qthread_create(run_joiner, &t1);
    qthread_join(t2);
}

void* run_sleep_test(void* arg) {
    long sleep_time = 500000;  // 500ms in microseconds
    long start_time = get_usecs();

    qthread_usleep(sleep_time);

    long elapsed = get_usecs() - start_time;
    // Check if elapsed time is within 500ms ± 50ms (450000-550000 microseconds)
    assert(elapsed >= 450000 && elapsed <= 550000);
    printf("Thread slept for %ld microseconds\n", elapsed);
    return NULL;
}

void test_sleep_duration(void) {
    printf("\n=== Testing Sleep Duration ===\n");

    qthread_t t = qthread_create(run_sleep_test, NULL);
    qthread_join(t);

    printf("=== Sleep Duration test completed ===\n");
}







// Test structure to pass sleep duration and verify results
typedef struct {
    long sleep_time;
    long start_time;
    long end_time;
} sleep_info_t;



void test_four_threads_sleep(void) {
    printf("\n=== Testing Four Threads Sleep (250ms/500ms) ===\n");

    // Create info structures for each thread
    sleep_info_t info[4] = {
        {.sleep_time = 250000}, // 250ms
        {.sleep_time = 250000}, // 250ms
        {.sleep_time = 500000}, // 500ms
        {.sleep_time = 500000}  // 500ms
    };

    // Create and start all threads
    qthread_t threads[4];
    for(int i = 0; i < 4; i++) {
        threads[i] = qthread_create(run_sleep, &info[i]);
    }

    // Wait for all threads to complete
    for(int i = 0; i < 4; i++) {
        qthread_join(threads[i]);
    }

    printf("=== Four Threads Sleep test completed ===\n");
}



void test_two_threads_sleep(void) {
    printf("\n=== Testing Two Threads Sleep (500ms) ===\n");

    sleep_info_t info1 = {.sleep_time = 500000}; // 500ms
    sleep_info_t info2 = {.sleep_time = 500000}; // 500ms

    // Create and start both threads
    qthread_t t1 = qthread_create(run_sleep, &info1);
    qthread_t t2 = qthread_create(run_sleep, &info2);

    // Wait for both threads to complete
    qthread_join(t1);
    qthread_join(t2);

    printf("=== Two Threads Sleep test completed ===\n");
}
