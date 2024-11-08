#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "qthread.h"

#define NUM_THREADS 4
#define STACK_SIZE 64*1024

extern void *setup_stack(void *_stack, size_t len, f_2arg_t f, f_1arg_t f2, void *arg);
extern void switch_thread(void **location_for_old_sp, void *new_value);
//GLOBALS
struct threadq active;
struct qthread current;
//
/* this is your qthread structure. */
struct qthread {
    struct qthread* next;
    void *saved_stack_pointer;
	// required later to free stack data in the heap
	void *stack_low_pointer;
};


/*
 * You'll probably want to define a thread queue structure, and
 * functions to append and remove threads. (Note that you only need to
 * remove the oldest item from the head, makes removal a lot easier)
 *
 * this later comes to be the active queue, cond var and mutex queue
 */
struct threadq {
    qthread_t front;
    qthread_t end;
    int size;
	// pointers to functions
    void (*push_back)(struct threadq *queue,struct qthread *thread);
    struct qthread (*pop_front)(struct threadq *queue);
};

void push_back(struct threadq *queue,struct qthread *thread)
{
	thread->next = queue->end;
	queue->end = thread;
	queue->size = queue->size + 1;
	printf("Enqueued thread %p\n",thread);
}

struct qthread pop_front(struct threadq *queue)
{
	if(queue->size == 0){
		perror("Empty queue to dequeue\n");
	}
	qthread_t head = queue->front;
	// iterate till you find new head
	qthread_t new_head = queue->end;
	while (new_head->next->next != NULL) {
		new_head = new_head->next;
	}
	queue->front = new_head;
	queue->size = queue -> size - 1;
	return *head;
}

int isEmpty(struct threadq* q) {
	if(q->front == q->end && q->end == NULL) {
		return 1;
	}
	return 0;
}

/**********/
/* CREATE */
/**********/
// TODO: check types
void create_thread_wrapper(f_1arg_t f, void *arg1)
{
	void *val = f(arg1);
	return qthread_exit(val);
}

/*
 * 1. creates a stack and sets it up with create_thread_wrapper in the stack
 * whenever the stack is executed it will call the wrapper func
 * 2. create thread wrapper calls f(arg1) and exits the thread there
 *
 * calling function must add this to RUNNABLE queue
*/
qthread_t qthread_create(f_1arg_t f, void *arg1)
{
	// 1. setup stack with wrapper function
	void *stack = malloc(STACK_SIZE);
	if (!stack) {
		perror("Failed to allocate stack");
		exit(1);
	}
	void *sp = setup_stack(stack, STACK_SIZE, create_thread_wrapper, f, arg1);

	// 2. create the actual thread
	struct qthread *thread = malloc(sizeof(qthread_t));
	if (!thread) {
		perror("Failed to allocate memory for qthread in qthread_create");
		exit(1);
	}
	thread->saved_stack_pointer = sp;
	thread->next = NULL;

	// 3. TODO: make the thread runnable ++ add it to the queue

	return thread;
}

/* qthread_init - set up a thread structure for the main (OS-provided) thread
 */
void qthread_init(void)
{
	// create a thread for the main running thread and set it as active
	// QUESTION: how to deal with the stack pointer here
	struct qthread *thread = malloc(sizeof(qthread_t));
	if (!thread) {
		perror("Failed to allocate memory for qthread in qthread_init");
		exit(1);
	}

	// create the RUNNABLE QUEUE
	struct threadq *runnable_queue = malloc(sizeof(struct threadq));
	/* runnable_queue->push_back(t) */
}

/* I suggest factoring your code so that you have a 'schedule'
 * function which selects the next thread to run and @switches to it,
 * or goes to sleep if there aren't any threads left to run.
 *
 * NOTE - if you end up switching back to the same thread, do *NOT*
 * use do_switch - check for this case and return from schedule(),
 * or else @you'll crash.
 */
void schedule(){

}



/* qthread_yield - yield to the next @runnable thread.
 */
void qthread_yield(void)
{
  if(active.size == 0){
    return;
  }
  else{
    //logic here
    struct qthread tmp = current;
	push_back(&active,&current);
    current = pop_front(&active);
    schedule();
  }
}

/* qthread_exit, qthread_join - exit argument is returned by
 * qthread_join. Note that join blocks if the thread hasn't exited
 * yet, and is allowed to crash @if the thread doesn't exist.
 */
void qthread_exit(void *val)
{
	struct qthread tmp = current;
	push_back(&active,&current);
	current = pop_front(&active);
	switch_thread(&tmp.saved_stack_pointer, current.saved_stack_pointer);
	if(isEmpty(&active)){
		return;
	}
}
void *qthread_join(qthread_t thread)
{

}

