/* wait.c - wait */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <syscall_stats.h>

/*------------------------------------------------------------------------
 * wait  --  make current process wait on a semaphore
 *------------------------------------------------------------------------
 */
SYSCALL	wait(int sem)
{
	size_t start_time;
	if (syscall_statistician.enabled) {
		syscall_statistician.stats[currpid].count[SYS_WAIT]++;
		start_time = ctr1000;
	}

	STATWORD ps;    
	struct	sentry	*sptr;
	struct	pentry	*pptr;

	disable(ps);
	if (isbadsem(sem) || (sptr= &semaph[sem])->sstate==SFREE) {
		restore(ps);
		if (syscall_statistician.enabled) {
			size_t end_time = ctr1000;
			syscall_statistician.stats[currpid].total_time[SYS_WAIT] += end_time - start_time;
		}
		return(SYSERR);
	}
	
	if (--(sptr->semcnt) < 0) {
		(pptr = &proctab[currpid])->pstate = PRWAIT;
		pptr->psem = sem;
		enqueue(currpid,sptr->sqtail);
		pptr->pwaitret = OK;
		resched();
		restore(ps);

		if (syscall_statistician.enabled) {
			size_t end_time = ctr1000;
			syscall_statistician.stats[currpid].total_time[SYS_WAIT] += end_time - start_time;
		}

		return pptr->pwaitret;
	}
	restore(ps);

	if (syscall_statistician.enabled) {
		size_t end_time = ctr1000;
		syscall_statistician.stats[currpid].total_time[SYS_WAIT] += end_time - start_time;
	}
	
	return(OK);
}
