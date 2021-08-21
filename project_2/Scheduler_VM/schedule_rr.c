/* schedule.c
 * This file contains the primary logic for the 
 * scheduler.
 */
#include "schedule.h"
#include "macros.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

#define NEWTASKSLICE (NS_TO_JIFFIES(100000000))

/* Local Globals
 * rq - This is a pointer to the runqueue that the scheduler uses.
 * current - A pointer to the current running task.
 */
struct runqueue *rq;
struct task_struct *current;

/* External Globals
 * jiffies - A discrete unit of time used for scheduling.
 *			 There are HZ jiffies in a second, (HZ is 
 *			 declared in macros.h), and is usually
 *			 1 or 10 milliseconds.
 */
extern long long jiffies;
extern struct task_struct *idle;

/*-----------------Initilization/Shutdown Code-------------------*/
/* This code is not used by the scheduler, but by the virtual machine
 * to setup and destroy the scheduler cleanly.
 */
 
 /* initscheduler
  * Sets up and allocates memory for the scheduler, as well
  * as sets initial values. This function should also
  * set the initial effective priority for the "seed" task 
  * and enqueu it in the scheduler.
  * INPUT:
  * newrq - A pointer to an allocated rq to assign to your
  *			local rq.
  * seedTask - A pointer to a task to seed the scheduler and start
  * the simulation.
  */
void initschedule(struct runqueue *newrq, struct task_struct *seedTask)
{
	seedTask->next = seedTask->prev = seedTask;
	newrq->head = seedTask;
	newrq->nr_running++;
}

/* killschedule
 * This function should free any memory that 
 * was allocated when setting up the runqueu.
 * It SHOULD NOT free the runqueue itself.
 */
void killschedule()
{
	return;
}


void print_rq () {
	struct task_struct *curr;
	
	printf("Rq: \n");
	curr = rq->head;
	if (curr)
		printf("%p", curr);
	while(curr->next != rq->head) {
		curr = curr->next;
		printf(", %p", curr);
	};
	printf("\n");
}

/*-------------Scheduler Code Goes Below------------*/
/* This is the beginning of the actual scheduling logic */

/* schedule
 * Gets the next task in the queue
 */
void schedule()
{
	static struct task_struct *nxt = NULL;
	struct task_struct *curr;
	
//	printf("In schedule\n");
//	print_rq();
	
	current->need_reschedule = 0; /* Always make sure to reset that, in case *
								   * we entered the scheduler because current*
								   * had requested so by setting this flag   */
	
	if (rq->nr_running == 1) {
		context_switch(rq->head);
		nxt = rq->head->next;
	}
	else {	
		curr = nxt;
		nxt = nxt->next;
		if (nxt == rq->head)    /* Do this to always skip init at the head */
			nxt = nxt->next;	/* of the queue, whenever there are other  */
								/* processes available					   */
		context_switch(curr);
	}
}


/* sched_fork
 * Sets up schedule info for a newly forked task
 */
void sched_fork(struct task_struct *p)
{
	p->time_slice = 100;
}

/* scheduler_tick
 * Updates information and priority
 * for the task that is currently running.
 */
void scheduler_tick(struct task_struct *p)
{
	schedule();
}

/* wake_up_new_task
 * Prepares information for a task
 * that is waking up for the first time
 * (being created).
 */
void wake_up_new_task(struct task_struct *p)
{	
	p->next = rq->head->next;
	p->prev = rq->head;
	p->next->prev = p;
	p->prev->next = p;
	
	rq->nr_running++;
}

/* activate_task
 * Activates a task that is being woken-up
 * from sleeping.
 */
void activate_task(struct task_struct *p)
{
	p->next = rq->head->next;
	p->prev = rq->head;
	p->next->prev = p;
	p->prev->next = p;
	
	rq->nr_running++;
}

/* deactivate_task
 * Removes a running task from the scheduler to
 * put it to sleep.
 */
void deactivate_task(struct task_struct *p)
{
	p->prev->next = p->next;
	p->next->prev = p->prev;
	p->next = p->prev = NULL; /* Make sure to set them to NULL *
							   * next is checked in cpu.c      */

	rq->nr_running--;
}

struct task_struct *get_min_exp_burst(){
    
	/****************************************************************************/
	/* Calculate minimum expected burst*/
    
    // -----------   SOS    ----------
    //skip the head, because it runs forever
	double temp_min_burst;
	struct task_struct *curr, *min_burst;

	curr = rq->head->next;
	temp_min_burst = curr->exp_burst;

	//initiallize pointers to first process
	min_burst = curr;

    curr = curr->next;
	while(curr != rq->head){
		// find minimum
		if(temp_min_burst > curr->exp_burst){
			temp_min_burst = curr->exp_burst;	// Update min_burst var
			min_burst = curr;			// Point to new min exp_burst process
		}
        curr = curr->next;
	}
	return(min_burst);
    
}

struct task_struct *get_max_waitinRQ(){
    
	/****************************************************************************/
	/* Calculate maximum WaitingInRQ between processes */
	unsigned long long temp_max_waitInRQ;
	struct task_struct *curr, *max_waitInRQ;

	curr = rq->head->next;
	temp_max_waitInRQ = sched_clock() - curr->prev_in_queue;

	//initiallize pointers to first process
	max_waitInRQ = curr;

    curr = curr->next;
	while(curr != rq->head){
		// find minimum
		if(temp_max_waitInRQ < sched_clock() - curr->prev_in_queue){
			temp_max_waitInRQ = sched_clock()- curr->prev_in_queue;	// Update min_burst var
			max_waitInRQ = curr;			// Point to new min exp_burst process
		}
        curr = curr->next;
	}
	return(max_waitInRQ);
    
}

struct task_struct *get_goodness(){
    
    // Calculate minimum Goodness proccess
    struct task_struct *curr,*min_goodness,*min_exp_burst,*max_waitingRQ;
	curr = rq->head->next;
    min_goodness = curr;
    double temp_min_goodness = curr->goodness;
    min_exp_burst = get_min_exp_burst();
    max_waitingRQ = get_max_waitinRQ();
    while(curr != rq->head){

        curr->goodness = ((1 + curr->exp_burst)/(1+ min_exp_burst->exp_burst))*((1+ max_waitingRQ->prev_in_queue)/(1+((double) sched_clock() - (double) curr->prev_in_queue)));
		if(temp_min_goodness > curr->goodness){
			temp_min_goodness = curr->goodness;
			min_goodness = curr;
		}
		
		curr = curr->next;
	}
    return(min_goodness);
}
struct task_struct *get_standard_goodness(){
    
    // Calculate minimum Goodness proccess
    struct task_struct *curr,*min_sd_goodness,*min_exp_burst,*max_waitingRQ;
	curr = rq->head->next;
    min_sd_goodness = curr;
    double temp_min_sd_goodness = curr->sd_goodness;
    min_exp_burst = get_min_exp_burst();
    max_waitingRQ = get_max_waitinRQ();
    while(curr != rq->head){

        curr->sd_goodness = ((1 + curr->exp_burst)/(1+ min_exp_burst->exp_burst));
		if(temp_min_sd_goodness > curr->sd_goodness){
			temp_min_sd_goodness = curr->sd_goodness;
			min_sd_goodness = curr;
		}
		
		curr = curr->next;
	}
    return(min_sd_goodness);
} 
