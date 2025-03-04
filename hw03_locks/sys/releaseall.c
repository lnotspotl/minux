#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <lock.h>
#include <stdio.h>

/* External variables */
extern unsigned long ctr1000;  /* counts in 1000ths of second */

int lock_releaseall(int numlocks, int args, ...)
{
    STATWORD ps;
    int result = OK;

    // Error checking
    if (numlocks <= 0)
        return SYSERR;

    // Disable interrupts
    disable(ps);
    
    // Get pointer to the list of lock descriptors
    int *largs = (int *)&args;
    
    for (int i = 0; i < numlocks; i++) {
        // Release lock
        int release_result = lock_release(*largs++);


        // If one release fails, this entire operation fails
        if (release_result == SYSERR) {
            result = SYSERR;
        }
    }

    // Restore interrupts
    restore(ps);
    return result;
} 