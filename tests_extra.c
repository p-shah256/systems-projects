#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "qthread.h"
#include <sys/time.h>



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
    qthread_create(run_test2, "e")};
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
  assert(!strcmp(val, "a"));
  val = qthread_join(t[1]);
  assert(!strcmp(val, "b"));
  val = qthread_join(t[2]);
  assert(!strcmp(val, "c"));
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
