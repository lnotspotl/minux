/* gettime.c - gettime */

#include <conf.h>
#include <kernel.h>
#include <date.h>
#include <syscall_stats.h>
extern int getutim(unsigned long *);

/*------------------------------------------------------------------------
 *  gettime  -  get local time in seconds past Jan 1, 1970
 *------------------------------------------------------------------------
 */
SYSCALL	gettime(long *timvar)
{
	size_t start_time;

	if (syscall_statistician.enabled) {
		syscall_statistician.stats[currpid].count[SYS_GETTIME]++;
		start_time = ctr1000;
	}

	/* FIXME -- no getutim */

	if (syscall_statistician.enabled) {
		size_t end_time = ctr1000;
		syscall_statistician.stats[currpid].total_time[SYS_GETTIME] += end_time - start_time;
	}

    return OK;
}
