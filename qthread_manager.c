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
static long get_usecs(void);

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
	void *stack_low_pointer;
	void *return_val;
	int dead;
	long int timing_information;
	qthread_t waiter;
};

/*
 * exit = 0 if yeild
 *
 * exit = 1 if exit
 *
 * exit = 2 if wait/join
 *
 * exit = 3 if sleep
 */
void schedule(int exit);


/****************************************************************************************/
/* THREAD AND ITS OPERATIONS */
/****************************************************************************************/

threadq_t runnable_queue;
qthread_t current_thread;
threadq_t sleeping_set;

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
	void *stack = malloc(STACK_SIZE);
	if (!stack) {
		perror("Failed to allocate stack");
		exit(1);
	}
	void *sp = setup_stack(stack, STACK_SIZE, create_thread_wrapper, f, arg1);
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
	return thread;
}

/* qthread_init - set up a thread structure for the main (OS-provided) thread
 */
void qthread_init(void)
{
	qthread_t thread = malloc(sizeof(struct qthread));
	if (!thread) {
		perror("Failed to allocate memory for qthread in qthread_init");
		exit(1);
	}

	runnable_queue = malloc(sizeof(struct threadq));
    runnable_queue->front = NULL;
    runnable_queue->end = NULL;
    runnable_queue->size = 0;
	// SLEEPING SET
	sleeping_set = malloc(sizeof(struct threadq));
    sleeping_set->front = NULL;
    sleeping_set->end = NULL;
    sleeping_set->size = 0;
	current_thread = thread;
}


/* qthread_yield - yield to the next @runnable thread.
 */
void qthread_yield(void)
{
	schedule(0);
}



void switch_runnable(qthread_t old_current) {
	if (runnable_queue->size == 0) {
	}
	current_thread = pop_front(runnable_queue);
	if (current_thread == old_current) {
		return;
	}
	if(current_thread->dead != 1){
	    switch_thread(&(old_current->saved_stack_pointer), (current_thread->saved_stack_pointer));
	}
	return;
}

static int wake_sleeping_threads(void) {
    if (sleeping_set->size == 0) {
        return 0;  // no sleeping threads
    }
    qthread_t head = pop_front(sleeping_set);

    while (head->timing_information > get_usecs()) {
        if (head->next) {
            qthread_t new_head = pop_front(sleeping_set);
            push_back(sleeping_set, head);
            head = new_head;
        }
    }

    push_back(runnable_queue, head);

    return 1;  // successfully woke a thread
}

void schedule(int exit)
{
	qthread_t old_current = current_thread;

	if (runnable_queue->size == 0 && exit == 1) {
		if (sleeping_set->size > 0) {
			wake_sleeping_threads(); // only if sleeping threads present
		}
	}

	if (exit == 1) { // EXIT
	} else if (exit == 2) { // JOIN
		// don't push current to runnable .... its waiting for some other thread to end
	} else if (exit == 3) { // SLEEP
		// move it into sleeping set
		push_back(sleeping_set, old_current);
	} else {                // YEILD
		// if last thread calls yeild, crash .. simpler
		push_back(runnable_queue, old_current);
	}

	if (runnable_queue->size == 0 ) {
		if (sleeping_set->size > 0) {
			wake_sleeping_threads(); // only if sleeping threads present
		}
	}

	switch_runnable(old_current);
}

/* qthread_exit, qthread_join - exit argument is returned by
* qthread_join. Note that join blocks if the thread hasn't exited
* yet, and is allowed to crash @if the thread doesn't exist.
* return;
*/
void qthread_exit(void *val)
{
	current_thread->return_val = val;
	current_thread->dead = 1;
	if (current_thread->waiter) {
		push_back(runnable_queue, current_thread->waiter);
	}
	schedule(1);
	return;
}

void *qthread_join(qthread_t thread)
{
	thread->waiter = current_thread;
	while (thread->dead != 1) {
		schedule(2); // 2= wait - does not put it into runnable_queue will be woken up by some other thread
	}
	return thread->return_val;
}


/* qthread_usleep - yield to next runnable thread, making arrangements
* to be put back on the active list after 'usecs' timeout.
*/
void qthread_usleep(long int usecs)
{
	current_thread->timing_information = get_usecs() + usecs;
	schedule(3);
}


/************************************************************************************************/
/* QUEUE AND OPERATIONS */
/************************************************************************************************/
void push_back(threadq_t queue, qthread_t thread)
{
    thread->next = NULL;
    if (queue->size == 0) {
        queue->front = thread;
        queue->end = thread;
    }
    else {
        queue->end->next = thread;
        queue->end = thread;
    }
    queue->size++;
}

qthread_t pop_front(struct threadq *queue)
{
    if(queue->size == 0) {
		perror("cannot pop from an empty queue");
		return NULL;
    }
    qthread_t head = queue->front;
    queue->front = head->next;
    queue->size--;

    if (queue->size == 0) {
        queue->end = NULL;
    }

    return head;
}
