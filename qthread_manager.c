#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "qthread.h"

#define NUM_THREADS 4
#define STACK_SIZE 64*1024

/********************************************************************************/
/* DECLARATIONS */
/********************************************************************************/
extern void *setup_stack(void *_stack, size_t len, f_2arg_t f, f_1arg_t f2, void *arg);
extern void switch_thread(void **location_for_old_sp, void *new_value);

/*
 * You'll probably want to define a thread queue structure, and
 * functions to append and remove threads. (Note that you only need to
 * remove the oldest item from the head, makes removal a lot easier)
 *
 * this later comes to be the active queue, cond var and mutex queue
 */
struct threadq
{
    qthread_t front;
    qthread_t end;
    int size;
};

typedef struct threadq *threadq_t;
void push_back(threadq_t queue, qthread_t thread);
qthread_t pop_front(struct threadq *queue);

/* this is your qthread structure. */
struct qthread
{
    struct qthread* next;
    void *saved_stack_pointer;
	// required later to free stack data in the heap
	void *stack_low_pointer;
	// to store return value
	void *return_val;
	// to signal dead
	int dead;
	long int timing_information;
	// for join
	qthread_t waiter;
};

/* I suggest factoring your code so that you have a 'schedule'
 * function which selects the next thread to run and @switches to it,
 * or goes to sleep if there aren't any threads left to run.
 *
 * NOTE - if you end up switching back to the same thread, do *NOT*
 * use do_switch - check for this case and return from schedule(),
 * or else @you'll crash.
 *
 * exit = 1 if qthread_exit
 * exit = 0 if qthread_yeild
 */
void schedule(int exit);


/****************************************************************************************/
/* THREAD AND ITS OPERATIONS */
/****************************************************************************************/

threadq_t runnable_queue;
qthread_t current_thread;

// TODO: check types
void create_thread_wrapper(f_1arg_t f, void *arg1)
{
	printf("\n%p THREAD WRAPPER: running thread\n", current_thread);
	void *val = f(arg1);
	printf("\n%p THREAD WRAPPER: exiting thread returning: %p (value: %s)\n", current_thread, val, (char*)val);
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
	printf("\nCREATE: creating...\n");
	// 1. setup stack with wrapper function
	void *stack = malloc(STACK_SIZE);
	if (!stack) {
		perror("Failed to allocate stack");
		exit(1);
	}
	void *sp = setup_stack(stack, STACK_SIZE, create_thread_wrapper, f, arg1);

	// 2. create the actual thread
	/* struct qthread *thread = malloc(sizeof(qthread_t)); */
	struct qthread *thread = malloc(sizeof(struct qthread));  // CORRECT!
	if (!thread) {
		perror("Failed to allocate memory for qthread in qthread_create");
		exit(1);
	}
	thread->saved_stack_pointer = sp;
	thread->next = NULL;
	thread->dead = 0;
	thread->waiter = NULL;
	push_back(runnable_queue, thread);

	printf("CREATE: created \n");
	return thread;
}

/* qthread_init - set up a thread structure for the main (OS-provided) thread
 */
void qthread_init(void)
{
	// create a thread for the main running thread and set it as active
	// QUESTION: how to deal with the stack pointer here
	qthread_t thread = malloc(sizeof(struct qthread));
	if (!thread) {
		perror("Failed to allocate memory for qthread in qthread_init");
		exit(1);
	}

	// TODO: check out how to setup stack for this one
	// create the RUNNABLE QUEUE and mark as active

	printf("qthread system initialized \n");
	runnable_queue = malloc(sizeof(struct threadq));
	current_thread = thread;
}


/* qthread_yield - yield to the next @runnable thread.
 */
void qthread_yield(void)
{
	printf("\n%p YEILD: yeilding.... \n", current_thread);
	schedule(0);
}


void schedule(int exit)
{
	printf("%p SCHEDULE: schedule called\n", current_thread);
	qthread_t old_current = current_thread;
	// if no threads remain either go to sleep or crash?
	if (runnable_queue->size == 0) {
		printf("%p SCHEDULE: queue size 0, not yeilding\n", current_thread);
		return;
	}

	if (exit == 1) { // EXIT
		free(old_current);
	} else if (exit == 2) { // WAIT
	    // do not push back the current thread
	} else {         // YEILD
		push_back(runnable_queue, old_current);
	}

	current_thread = pop_front(runnable_queue);
	printf("%p SCHEDULE: all setup, switching from %p -> %p\n", old_current, old_current, current_thread);
	switch_thread(&(old_current->saved_stack_pointer), (current_thread->saved_stack_pointer));
	return;
}

/* qthread_exit, qthread_join - exit argument is returned by
 * qthread_join. Note that join blocks if the thread hasn't exited
 * yet, and is allowed to crash @if the thread doesn't exist.
 * return;
 */
void qthread_exit(void *val)
{
	printf("%p EXIT: exiting...\n", current_thread);
	current_thread->return_val = val;
	current_thread->dead = 1;
	// wake up any sleeping threads -- add them to the runnable list
	if (current_thread->waiter) {
		printf("%p EXIT: waking up sleeping thread %p\n", current_thread, current_thread->waiter);
		push_back(runnable_queue, current_thread->waiter);
	}
	schedule(1);
	return;
}

void *qthread_join(qthread_t thread)
{
	printf("\n%p JOIN: is joining and waiting for %p\n", current_thread, thread);
	thread->waiter = current_thread;
	while (thread->dead != 1) {
		printf("%p JOIN: thread is not dead yet, putting to wait\n", current_thread);
		// and take it off runnable list too
		schedule(2); // 2= wait - does not put it into runnable_queue will be woken up by some other thread
	}
	return thread->return_val;
}


/* qthread_usleep - yield to next runnable thread, making arrangements
 * to be put back on the active list after 'usecs' timeout.
 */
void qthread_usleep(long int usecs)
{
}


/************************************************************************************************/
/* QUEUE AND OPERATIONS */
/************************************************************************************************/
void push_back(threadq_t queue, qthread_t thread)
{
    thread->next = NULL;
    // If queue is empty, both front and end should point to new thread
    if (queue->size == 0) {
        queue->front = thread;
        queue->end = thread;
    }
    // Otherwise, update the current end node to point to new thread
    else {
        queue->end->next = thread;
        queue->end = thread;
    }
    queue->size++;
    printf("QUEUE: Pushed back thread %p\n", thread);
}

qthread_t pop_front(struct threadq *queue)
{

    printf("QUEUE: pop_front returning %p\n", queue->front);
    // Check for empty queue
    if(queue->size == 0) {
		perror("cannot pop from an empty queue");
		return NULL;
    }
    qthread_t head = queue->front;
    queue->front = head->next;
    queue->size--;

    // If we just removed the last element, update end pointer
    if (queue->size == 0) {
        queue->end = NULL;
    }

    return head;
}

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
	/* your code here */
}
void *qthread_join(qthread_t thread)
{
	/* your code here */
}

