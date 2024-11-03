#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "qthread.h"

#define NUM_THREADS 4
#define STACK_SIZE 64*1024

extern void *setup_stack(void *_stack, size_t len, f_2arg_t f, f_1arg_t f2, void *arg);

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
