/* receive.c - receive */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <syscall_stats.h>
/*------------------------------------------------------------------------
 *  receive  -  wait for a message and return it
 *------------------------------------------------------------------------
 */
SYSCALL	receive()
{
	STATWORD ps;    
	struct	pentry	*pptr;
	WORD	msg;

	size_t start_time;

	if (syscall_statistician.enabled) {
		syscall_statistician.stats[currpid].count[SYS_RECEIVE]++;
		start_time = ctr1000;
	}

	disable(ps);
	pptr = &proctab[currpid];
	if ( !pptr->phasmsg ) {		/* if no message, wait for one	*/
		pptr->pstate = PRRECV;
		resched();
	}
	msg = pptr->pmsg;		/* retrieve message		*/
	pptr->phasmsg = FALSE;
	restore(ps);

	if (syscall_statistician.enabled) {
		size_t end_time = ctr1000;
		syscall_statistician.stats[currpid].total_time[SYS_RECEIVE] += end_time - start_time;
	}

	return(msg);
}
