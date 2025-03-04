/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;  
	disable(ps);

	int ret = chprio_clean(pid, newprio);
	if(ret != SYSERR) {
		run_priority_inheritance_algo();
	}

	restore(ps);
	return ret;

}

SYSCALL chprio_clean(int pid, int newprio) {
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 || (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pprio = newprio;
	pptr->original_priority = newprio;
	restore(ps);
	return(newprio);
}