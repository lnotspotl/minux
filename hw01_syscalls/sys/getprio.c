/* getprio.c - getprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <syscall_stats.h>
/*------------------------------------------------------------------------
 * getprio -- return the scheduling priority of a given process
 *------------------------------------------------------------------------
 */
SYSCALL getprio(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;
	size_t start_time;

	if (syscall_statistician.enabled) {
		syscall_statistician.stats[currpid].count[SYS_GETPRIO]++;
		start_time = ctr1000;
	}

	disable(ps);
	if (isbadpid(pid) || (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		if (syscall_statistician.enabled) {
			size_t end_time = ctr1000;
			syscall_statistician.stats[currpid].total_time[SYS_GETPRIO] += end_time - start_time;
		}
		return(SYSERR);
	}
	restore(ps);

	if (syscall_statistician.enabled) {
		size_t end_time = ctr1000;
		syscall_statistician.stats[currpid].total_time[SYS_GETPRIO] += end_time - start_time;
	}

	return(pptr->pprio);
}

