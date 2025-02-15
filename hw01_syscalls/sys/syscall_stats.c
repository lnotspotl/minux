#include <kernel.h>
#include <proc.h>

struct perprocesssyscallstats syscall_statistician;

void syscallsummary_start(void) {
    // Enable syscall statistics collection
	syscall_statistician.enabled = TRUE;

    // Reset all counts and total times
    int i, j;
    for (i = 0; i < NPROC; i++) {
        for (j = 0; j < NUM_SYSCALLS; j++) {
            syscall_statistician.stats[i].count[j] = 0;
            syscall_statistician.stats[i].total_time[j] = 0;
        }
    }
}

void syscallsummary_stop(void) {
    // Disable syscall statistics collection
	syscall_statistician.enabled = FALSE;
}

void printsyscallsummary() {
    int i, j;
    static char *syscall_names[] = {
        "freemem", "chprio", "getpid", "getprio", "gettime", "kill",
        "receive", "recvclr", "recvtim", "resume", "scount", "sdelete",
        "send", "setdev", "setnok", "screate", "signal", "signaln",
        "sleep", "sleep10", "sleep100", "sleep1000", "sreset", "stacktrace",
        "suspend", "unsleep", "wait"
    };

    for (i = 0; i < NPROC; i++) {
        struct pentry *proc = &proctab[i];
        if (proc->pstate == PRFREE)
            continue;
            
        int has_syscalls = 0;
        for (j = 0; j < NUM_SYSCALLS; j++) {
            if (syscall_statistician.stats[i].count[j] > 0) {
                if (!has_syscalls) {
                    kprintf("\nProcess [pid:%d]\n", i);
                    has_syscalls = 1;
                }
                size_t avg_time = 0;
                if (syscall_statistician.stats[i].count[j] > 0) {
                    avg_time = syscall_statistician.stats[i].total_time[j] / 
                              syscall_statistician.stats[i].count[j];
                    kprintf("    Syscall: %s, count: %d, average execution time: %d (ms)\n",
                    syscall_names[j],
                    syscall_statistician.stats[i].count[j],
                    avg_time);
                }
            }
        }
    }
    kprintf("\n");
}