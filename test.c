/*
 * file:        test.c
 * description: basic unit tests for Lab 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "qthread.h"

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
}
    
int main(int argc, char** argv)
{
    qthread_init();
    test1();
}
