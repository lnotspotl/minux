/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>

int current_sched_class = AGESCHED;

LOCAL int resched_age();
LOCAL int resched_linux();
LOCAL int resched_xinu();

LOCAL int project_priority(int prio, int min, int max);

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	switch (getschedclass()) {
		case AGESCHED:
			return resched_age();
		case LINUXSCHED:
			return resched_linux();
		case XINUSCHED:
			return resched_xinu();
		default:
			return SYSERR; // should never happen
	}
}

int resched_age() {
	register struct pentry *optr;  /* pointer to old process entry */
	register struct pentry *nptr;  /* pointer to new process entry */
	int pid;
	int next_prio;

	// Increment the priority of all processes in the ready queue
	for (pid = q[rdyhead].qnext; pid != rdytail; pid = q[pid].qnext) {

		// Null process should be ignored
		if(pid == 0) {
			continue;
		}

		// Increment priority and project it to valid range
		q[pid].qkey = project_priority(q[pid].qkey + 2, MINPRIO, MAXPRIO);
	}

	// Check the priority of the currently running process. 
	// No need to perform context switch if it has the highest priority
	// If equal, perform context switch (round robin)
	optr = &proctab[currpid];
	if (optr->pstate == PRCURR && lastkey(rdytail) < optr->pprio) {
		return (OK);
	}

	// Change the state of the current process to PRREADY and insert it into the ready queue
	if(optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid, rdyhead, optr->pprio);
	}

	// Remove the highest priority process at the end of the ready queue
	currpid = getlast(rdytail);
	nptr = &proctab[currpid];

	// Mark this new process as currently running
	nptr->pstate = PRCURR;  

#ifdef RTCLOCK
	preempt = QUANTUM; // reset preemption counter
#endif

	// Perform context switch
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

	// All done! Success
	return OK;
}

int resched_xinu() {
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	/* no switch needed if current process priority higher than next*/

	if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   (lastkey(rdytail)<optr->pprio)) {
		return(OK);
	}
	
	/* force context switch */

	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest priority process at end of ready list */

	nptr = &proctab[ (currpid = getlast(rdytail)) ];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
#endif
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}

int resched_linux() {
	return OK;
}

void setschedclass(int sched_class) {
	current_sched_class = sched_class;
}

int getschedclass() {
	return current_sched_class;
}

int project_priority(int priority, int min, int max) {

	// If priority is greater than max, return max
	if(priority > max) {
		return max;
	}

	// If priority is less than min, return min
	if(priority < min) {
		return min;
	}

	// Otherwise, return priority
	return priority;
}
