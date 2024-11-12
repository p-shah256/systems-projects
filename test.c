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

static long get_usecs(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000000 + tv.tv_usec;
}

/*
  create/yield/join/exit, make sure you test the following cases:
    1 thread, yield several times, then exit
    2 threads, yield back and forth a few times, both exit
    3 or more threads, same
    call join before the child thread exits
    call join after the child thread exits (you may want to use qthread_usleep for this)
*/
void* run_test1(void* arg)
{
    int i;
    for (i = 0; i < 3; i++) {
        qthread_yield();
        printf("%s\n", (char*)arg);
    }
    return arg;
}

void test1(void)
{
    qthread_t t = qthread_create(run_test1, "1");
    void *val = qthread_join(t);
    assert(!strcmp(val, "1"));

    qthread_t t2[2] = {qthread_create(run_test1, "1"),
                       qthread_create(run_test1, "2")};
    val = qthread_join(t2[0]);
    assert(!strcmp(val, "1"));
    val = qthread_join(t2[1]);
    assert(!strcmp(val, "2"));

    qthread_t t3[3] = {qthread_create(run_test1, "1"),
                       qthread_create(run_test1, "2"),
                       qthread_create(run_test1, "3")};
    val = qthread_join(t3[0]);
    assert(!strcmp(val, "1"));
    val = qthread_join(t3[1]);
    assert(!strcmp(val, "2"));
    val = qthread_join(t3[2]);
    assert(!strcmp(val, "3"));
}

void* run_test2(void* arg){
  int i = 3;
  while(i > 0){
    i-=1;
    qthread_yield();
    printf("while loop pass %s\n", (char*)arg);
  }
  return arg;
}
void test2(void){
    qthread_t t = qthread_create(run_test2, "a");
    void *val = qthread_join(t);
    assert(!strcmp(val, "a"));
    qthread_t t2[3] = {qthread_create(run_test2, "b"),
                        qthread_create(run_test2, "c"),
                        qthread_create(run_test2, "a")};
    val = qthread_join(t2[0]);
    assert(!strcmp(val, "b"));
    val = qthread_join(t2[1]);
    assert(!strcmp(val, "c"));
    val = qthread_join(t2[2]);
    assert(!strcmp(val, "a"));

    // test null void *, should return just e?
    qthread_t t3[2] = {qthread_create(run_test2, "d"),
    qthread_create(NULL, "e")};
    val = qthread_join(t3[0]);
    assert(!strcmp(val, "d"));
    val = qthread_join(t3[1]);
    assert(!strcmp(val, "e"));
}

//globals for testing
int test3Count = 0;
qthread_mutex_t *mutex;
qthread_cond_t *cond;

//for mutex and condition testing
//for testing mutex we must test conditionals
void* run_test3(void* arg) {
  qthread_mutex_lock(mutex);
  printf("locking thread %s\n", (char*)arg);
  while(test3Count < 2){
      test3Count++;
      printf("while loop pass %s\n", (char*)arg);
      qthread_cond_wait(cond,mutex);
  }
    qthread_cond_signal(cond);
    printf("unlocking thread %s\n", (char*)arg);
    qthread_mutex_unlock(mutex);
    return arg;
}

//tests two threads running through a while loop that locks an operation
void test3(void){
   mutex = qthread_mutex_create();
   cond = qthread_cond_create();
   qthread_t t = qthread_create(run_test3, "a");
   qthread_t t2 = qthread_create(run_test3, "b");
   void *val = qthread_join(t);
   assert(!strcmp(val, "a"));
   val = qthread_join(t2);
   assert(!strcmp(val, "b"));
   qthread_mutex_destroy(mutex);
   qthread_cond_destroy(cond);
}

void* run_test4(void* arg){
  qthread_mutex_lock(mutex);
  printf("locking thread %s\n", (char*)arg);
  return arg;
}
//tests for locking a thread
void test4(void){
  mutex = qthread_mutex_create();
  //cond = qthread_cond_create();
  qthread_t t = qthread_create(run_test4, "a");
  void* val = qthread_join(t);
  assert(!strcmp(val, "a"));
}
//tests lock and unlock for two threads
void* run_test5(void *arg){
  qthread_mutex_lock(mutex);
  printf("locking thread %s\n", (char*)arg);
  qthread_mutex_unlock(mutex);
  printf("unlocking thread %s\n", (char*)arg);
  return arg;
}
void test5(void){
  mutex = qthread_mutex_create();
  qthread_t t[2] = {qthread_create(run_test5, "b"),
                     qthread_create(run_test5, "c")};
  void* val = qthread_join(t[0]);
  assert(!strcmp(val, "b"));
  val = qthread_join(t[1]);
  assert(!strcmp(val, "c"));
  qthread_mutex_destroy(mutex);

}
//test 3 threads locking and unlcoking
void* run_test6(void* arg){
  int i = 3;
  qthread_mutex_lock(mutex);
  printf("locking thread %s\n", (char*)arg);
  while(i > 0){
      i--;
      printf("while loop pass %s\n", (char*)arg);
      printf("value of i is %d\n", i);
  }
  printf("unlocking thread %s\n", (char*)arg);
  qthread_mutex_unlock(mutex);
  return arg;
}
void test6(void){
    mutex = qthread_mutex_create();
    qthread_t t[3] = {qthread_create(run_test6, "a"),
    qthread_create(run_test6, "b"),
    qthread_create(run_test6, "c")};
    void *val = qthread_join(t[0]);
    assert(!strcmp(val, "a"));
    val = qthread_join(t[1]);
    assert(!strcmp(val, "b"));
    val = qthread_join(t[2]);
    assert(!strcmp(val, "c"));
    qthread_mutex_destroy(mutex);
    qthread_cond_destroy(cond);
}
//test three threads with Condition Variable, waiting and signaling
void* run_test7(void* arg){
    int i = 0;
    qthread_mutex_lock(mutex);
    printf("locking thread %s\n", (char*)arg);
    while(i < 3){
        i++;
        printf("while loop pass %s , %d\n", (char*)arg,i);
        printf("waiting thread %s\n", (char*)arg);
        qthread_cond_wait(cond,mutex);
    }
    printf("signaling thread %s\n", (char*)arg);
    qthread_cond_signal(cond);
    printf("unlocking thread %s\n", (char*)arg);
    qthread_mutex_unlock(mutex);
    return arg;
}
void test7(void){
  mutex = qthread_mutex_create();
  cond = qthread_cond_create();
  qthread_t t[3] = {qthread_create(run_test7, "a"),
  qthread_create(run_test7, "b"),
  qthread_create(run_test7, "c")};
  void *val = qthread_join(t[0]);
  assert(!strcmp(val, "c"));
  val = qthread_join(t[1]);
  assert(!strcmp(val, "b"));
  val = qthread_join(t[2]);
  assert(!strcmp(val, "a"));
  qthread_mutex_destroy(mutex);
  qthread_cond_destroy(cond);
}
//tests three threads and wakes them all up at once
void* run_test8(void* arg){
    int i = 0;
    qthread_mutex_lock(mutex);
    printf("locking thread %s\n", (char*)arg);
    while(i < 3){
        i++;
        printf("while loop pass %s , %d\n", (char*)arg,i);
        printf("waiting thread %s\n", (char*)arg);
        qthread_cond_wait(cond,mutex);
    }
    printf("signaling thread %s\n", (char*)arg);
    qthread_cond_broadcast(cond);
    printf("unlocking thread %s\n", (char*)arg);
    qthread_mutex_unlock(mutex);
    return arg;
}
void test8(void){
    mutex = qthread_mutex_create();
    cond = qthread_cond_create();
    qthread_t t[3] = {qthread_create(run_test8, "a"),
    qthread_create(run_test8, "b"),
    qthread_create(run_test8, "c")};
    void *val = qthread_join(t[0]);
    assert(!strcmp(val, "a"));
    val = qthread_join(t[1]);
    assert(!strcmp(val, "b"));
    val = qthread_join(t[2]);
    assert(!strcmp(val, "c"));
    qthread_mutex_destroy(mutex);
    qthread_cond_destroy(cond);
}
// runs condition wait and signal twice for the threads
void* run_test9(void* arg){
    int i = 0;
    qthread_mutex_lock(mutex);
    printf("locking thread %s\n", (char*)arg);
    while(i < 3){
        i++;
        printf("while loop pass %s , %d\n", (char*)arg,i);
        printf("waiting thread %s\n", (char*)arg);
        qthread_cond_wait(cond,mutex);
    }
    printf("signaling thread %s\n", (char*)arg);
    qthread_cond_signal(cond);
    i = 0;
    while(i < 3){
        i++;
        printf("while loop 2 pass %s , %d\n", (char*)arg,i);
        printf("waiting 2 thread %s\n", (char*)arg);
        qthread_cond_wait(cond,mutex);
    }
    printf("signaling 2 thread %s\n", (char*)arg);
    qthread_cond_signal(cond);
    printf("unlocking thread %s\n", (char*)arg);
    qthread_mutex_unlock(mutex);
    return arg;
}
void test9(void){
    mutex = qthread_mutex_create();
    cond = qthread_cond_create();
    qthread_t t[3] = {qthread_create(run_test9, "a"),
    qthread_create(run_test9, "b"),
    qthread_create(run_test9, "c")};
    void *val = qthread_join(t[0]);
    assert(!strcmp(val, "a"));
    val = qthread_join(t[1]);
    assert(!strcmp(val, "b"));
    val = qthread_join(t[2]);
    assert(!strcmp(val, "c"));
    qthread_mutex_destroy(mutex);
    qthread_cond_destroy(cond);
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


qthread_mutex_t* mutex;
qthread_cond_t* cond;

void* run_thread1(void* arg) {
    qthread_mutex_lock(mutex);
	// does some work
    printf("Thread 1: 1 2 3\n");
	// assume pipe is full
    qthread_cond_wait(cond, mutex);

	// woken up here
    printf("Thread 1: Queue emptied\n");
    printf("Thread 1: 5 6\n");
    qthread_cond_signal(cond);
    qthread_mutex_unlock(mutex);
    return NULL;
}

void* run_thread2(void* arg) {
    qthread_mutex_lock(mutex);
	// does some work when it gets a chance
    printf("Thread 2: 3 2 1\n");
    qthread_cond_signal(cond);
	// queue is empty
    qthread_cond_wait(cond, mutex);

    printf("Thread 2: 6 5\n");
    qthread_mutex_unlock(mutex);
    return NULL;
}

void test_condvar(void) {
    printf("\n=== Testing Condition Variables ===\n");

    // Initialize mutex and condition variable
    mutex = qthread_mutex_create();
    cond = qthread_cond_create();

    // Create threads
    qthread_t t1 = qthread_create(run_thread1, NULL);
    qthread_t t2 = qthread_create(run_thread2, NULL);

    // Wait for both threads to complete
    qthread_join(t1);
    qthread_join(t2);

    // Cleanup
    qthread_mutex_destroy(mutex);
    qthread_cond_destroy(cond);

    printf("=== Condition Variable test completed ===\n");
}


int main(int argc, char** argv)
{
    qthread_init();
    /* test1(); */
    // test2();
    /* test7(); */
	test_timed_sleep();
	test_basic_sleep();
	test_join_sleep();
	test_condvar();
	test8();
	test9();
}
