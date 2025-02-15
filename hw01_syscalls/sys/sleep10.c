/* sleep10.c - sleep10 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include <syscall_stats.h>

/*------------------------------------------------------------------------
 * sleep10  --  delay the caller for a time specified in tenths of seconds
 *------------------------------------------------------------------------
 */
SYSCALL	sleep10(int n)
{
	STATWORD ps;    
	size_t start_time;

	if (syscall_statistician.enabled) {
		syscall_statistician.stats[currpid].count[SYS_SLEEP10]++;
		start_time = ctr1000;
	}

	if (n < 0  || clkruns==0) {
		if (syscall_statistician.enabled) {
			size_t end_time = ctr1000;
			syscall_statistician.stats[currpid].total_time[SYS_SLEEP10] += end_time - start_time;
		}
		return(SYSERR);
	}
	disable(ps);
	if (n == 0) {		/* sleep10(0) -> end time slice */
	        ;
	} else {
		insertd(currpid,clockq,n*100);
		slnempty = TRUE;
		sltop = &q[q[clockq].qnext].qkey;
		proctab[currpid].pstate = PRSLEEP;
	}
	resched();
        restore(ps);

	if (syscall_statistician.enabled) {
		size_t end_time = ctr1000;
		syscall_statistician.stats[currpid].total_time[SYS_SLEEP10] += end_time - start_time;
	}

	return(OK);
}