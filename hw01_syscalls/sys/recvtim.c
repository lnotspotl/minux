/* recvtim.c - recvtim */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>
#include <syscall_stats.h>
/*------------------------------------------------------------------------
 *  recvtim  -  wait to receive a message or timeout and return result
 *------------------------------------------------------------------------
 */
SYSCALL	recvtim(int maxwait)
{
	STATWORD ps;    
	struct	pentry	*pptr;
	int	msg;
	size_t start_time;

	if (syscall_statistician.enabled) {
		syscall_statistician.stats[currpid].count[SYS_RECVTIM]++;
		start_time = ctr1000;
	}

	if (maxwait<0 || clkruns == 0)
		return(SYSERR);
	disable(ps);
	pptr = &proctab[currpid];
	if ( !pptr->phasmsg ) {		/* if no message, wait		*/
	        insertd(currpid, clockq, maxwait*1000);
		slnempty = TRUE;
		sltop = (int *)&q[q[clockq].qnext].qkey;
	        pptr->pstate = PRTRECV;
		resched();
	}
	if ( pptr->phasmsg ) {
		msg = pptr->pmsg;	/* msg. arrived => retrieve it	*/
		pptr->phasmsg = FALSE;
	} else {			/* still no message => TIMEOUT	*/
		msg = TIMEOUT;
	}
	restore(ps);

	if (syscall_statistician.enabled) {
		size_t end_time = ctr1000;
		syscall_statistician.stats[currpid].total_time[SYS_RECVTIM] += end_time - start_time;
	}

	return(msg);
}
