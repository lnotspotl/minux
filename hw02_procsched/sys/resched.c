/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>

int current_sched_class = AGESCHED;

LOCAL int resched_age();
LOCAL int resched_linux();
LOCAL int resched_xinu();

LOCAL int project(int num, int min, int max);

unsigned long currSP; /* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched() {
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
  register struct pentry *optr; /* pointer to old process entry */
  register struct pentry *nptr; /* pointer to new process entry */
  int pid;
  int next_prio;

  // Increment the priority of all processes in the ready queue
  for (pid = q[rdyhead].qnext; pid != rdytail; pid = q[pid].qnext) {

    // Null process should be ignored
    if (pid == 0) {
      continue;
    }

    // Increment priority and project it to valid range
    // Note that this won't change the order in the queue as all the keys
    // are incremented by the same amount
    q[pid].qkey = project(q[pid].qkey + 2, MINPRIO, MAXPRIO);
  }

  // Check the priority of the currently running process.
  // No need to perform context switch if it has the highest priority
  // If equal, perform context switch (round robin)
  optr = &proctab[currpid];
  if (optr->pstate == PRCURR && lastkey(rdytail) < optr->pprio) {
    return (OK);
  }

  // Change the state of the current process to PRREADY and insert it into the
  // ready queue
  if (optr->pstate == PRCURR) {
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
  ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp,
        (int)nptr->pirmask);

  // All done! Success
  return OK;
}

int resched_xinu() {
  register struct pentry *optr; /* pointer to old process entry */
  register struct pentry *nptr; /* pointer to new process entry */

  /* no switch needed if current process priority higher than next*/

  if (((optr = &proctab[currpid])->pstate == PRCURR) &&
      (lastkey(rdytail) < optr->pprio)) {
    return (OK);
  }

  /* force context switch */

  if (optr->pstate == PRCURR) {
    optr->pstate = PRREADY;
    insert(currpid, rdyhead, optr->pprio);
  }

  /* remove highest priority process at end of ready list */

  nptr = &proctab[(currpid = getlast(rdytail))];
  nptr->pstate = PRCURR; /* mark it currently running	*/
#ifdef RTCLOCK
  preempt = QUANTUM; /* reset preemption counter	*/
#endif

  ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp,
        (int)nptr->pirmask);

  /* The OLD process returns here when resumed. */
  return OK;
}

extern unsigned long ctr1000;

int resched_linux() {
  register struct pentry *optr; /* pointer to old process entry */
  register struct pentry *nptr; /* pointer to new process entry */

  static int decrement = 1;
  static unsigned long last_time = 0;
  static int initialized = 0;
  static int epoch_end = 1;

  // Initialize the current time
  if (!initialized) {
    last_time = ctr1000;
    initialized = 1;
  }

  // Calculate the time difference between the current time and the last time
  int counter_decrement;
  if (decrement) {
    unsigned long current_time = ctr1000;
    unsigned long time_diff = current_time - last_time;
    counter_decrement = (int)(time_diff);
    last_time = current_time;
  }

  int best_pid;
  int pid;

  // Decrement the counter of the current process
  optr = &proctab[currpid];
  if (decrement) {
    optr->counter = project(optr->counter - counter_decrement, 0, MAXINT);
  }

  // Only null process flag
  int only_null_process = 1;

  /* Check if the current epoch has ended */
  if (epoch_end) {
    for (pid = 1; pid < NPROC; pid++) { // Start at 1, 0 is the null process

      // Skip non-existent processes
      if (proctab[pid].pstate == PRFREE) {
        continue;
      }

      // Calculate the new quantum for the process
      int new_quantum = (proctab[pid].counter / 2) + proctab[pid].pprio;
      proctab[pid].quantum = new_quantum;
      proctab[pid].counter = new_quantum;
      only_null_process = 0;
    }

    // Reset the epoch end flag
    epoch_end = 0;
  }

  /* Find the process with the best goodness */
  best_pid = -1;
  int best_goodness = 0;
  for (pid = 1; pid < NPROC; pid++) {
    if (proctab[pid].pstate == PRREADY || proctab[pid].pstate == PRCURR) {
      int goodness;
      if (proctab[pid].counter <= 0 || proctab[pid].quantum == 0) {
        goodness = 0; // Process has used up its quantum
      } else {
        goodness = proctab[pid].counter + proctab[pid].pprio;
      }

      only_null_process = 0;

      if (goodness > best_goodness) {
        best_goodness = goodness;
        best_pid = pid;
      }
    }
  }

  if (best_goodness == 0 && !only_null_process) {
    epoch_end = 1;
    decrement = 0;
    return resched_linux();
  }

  if (only_null_process) {
    best_pid = 0;
    epoch_end = 1;
  }

  /* If no suitable process is found, start a new epoch */
  if (best_pid == -1) {
    epoch_end = 1;
    decrement = 0;
    return resched_linux(); /* Recursive call to start new epoch */
  }

  if (currpid == best_pid && optr->pstate == PRCURR) {
    return OK;
  }

  /* Context switch is needed */
  nptr = &proctab[best_pid];

  /* If current process is running, move it to ready queue */
  if (optr->pstate == PRCURR) {
    optr->pstate = PRREADY;
  }

  /* Remove best process from ready queue and mark as running */
  nptr->pstate = PRCURR;
  currpid = best_pid;

#ifdef RTCLOCK
  preempt = QUANTUM; /* reset preemption counter */
#endif

  // Perform context switch
  ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp,
        (int)nptr->pirmask);

  // Reactivate counter decrement
  decrement = 1;

  // All done! Success
  return OK;
}

void setschedclass(int sched_class) { current_sched_class = sched_class; }

int getschedclass() { return current_sched_class; }

int project(int num, int min, int max) {

  // If priority is greater than max, return max
  if (num > max) {
    return max;
  }

  // If priority is less than min, return min
  if (num < min) {
    return min;
  }

  // Otherwise, return num
  return num;
}