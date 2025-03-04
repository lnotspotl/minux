#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int newlock();

int lock_create()
{
    STATWORD ps;
    int lock_index;
    
    disable(ps);

    lock_index = newlock();
    if (lock_index == SYSERR) {
        restore(ps);
        return SYSERR;
    }

    restore(ps);
    return lock_index;
}

LOCAL int newlock()
{
    for (int i = 0; i < NUM_LOCKS; ++i) {

        // Get next lock index
        int lock_index = next_lock_index++;
        if (next_lock_index >= NUM_LOCKS)
            next_lock_index = 0;
        
        // If the lock is free, use it and return its index
        if (locks[lock_index].state == LOCK_FREE) {
            locks[lock_index].state = LOCK_USED;
            locks[lock_index].type = 0; // this process can be either LOCK_READ or LOCK_WRITE - we don't know yet
            locks[lock_index].num_holding_processes = 0;
            locks[lock_index].max_lock_priority = 0;
            locks[lock_index].wait_time = 0;
            
            // No process is holding this lock
            for (int j = 0; j < NPROC; ++j) {
                locks[lock_index].process_is_holding[j] = FALSE;
            }
            
            return lock_index;
        }
    }

    // No free lock found, error out
    return SYSERR;
} 