/* setnok.c - setnok */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <syscall_stats.h>
/*------------------------------------------------------------------------
 *  setnok  -  set next-of-kin (notified at death) for a given process
 *------------------------------------------------------------------------
 */
SYSCALL	setnok(int nok, int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;
	size_t start_time;

	if (syscall_statistician.enabled) {
		syscall_statistician.stats[currpid].count[SYS_SETNOK]++;
		start_time = ctr1000;
	}

	disable(ps);
	if (isbadpid(pid)) {
		restore(ps);
		if (syscall_statistician.enabled) {
			size_t end_time = ctr1000;
			syscall_statistician.stats[currpid].total_time[SYS_SETNOK] += end_time - start_time;
		}
		return(SYSERR);
	}
	pptr = &proctab[pid];
	pptr->pnxtkin = nok;
	restore(ps);

	if (syscall_statistician.enabled) {
		size_t end_time = ctr1000;
		syscall_statistician.stats[currpid].total_time[SYS_SETNOK] += end_time - start_time;
	}

	return(OK);
}
