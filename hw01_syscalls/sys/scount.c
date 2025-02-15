/* scount.c - scount */

#include <conf.h>
#include <kernel.h>
#include <sem.h>
#include <syscall_stats.h>
/*------------------------------------------------------------------------
 *  scount  --  return a semaphore count
 *------------------------------------------------------------------------
 */
SYSCALL scount(int sem)
{
	extern	struct	sentry	semaph[];
	size_t start_time;

	if (syscall_statistician.enabled) {
		syscall_statistician.stats[currpid].count[SYS_SCOUNT]++;
		start_time = ctr1000;
	}

	if (isbadsem(sem) || semaph[sem].sstate==SFREE) {
		if (syscall_statistician.enabled) {
			size_t end_time = ctr1000;
			syscall_statistician.stats[currpid].total_time[SYS_SCOUNT] += end_time - start_time;
		}
		return(SYSERR);
	}

	if (syscall_statistician.enabled) {
		size_t end_time = ctr1000;
		syscall_statistician.stats[currpid].total_time[SYS_SCOUNT] += end_time - start_time;
	}

	return(semaph[sem].semcnt);
}
