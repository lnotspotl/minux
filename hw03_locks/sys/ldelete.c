#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

int lock_delete(int lock_descriptor)
{
    STATWORD ps;
    
    disable(ps);
    
    if (lock_isbad(lock_descriptor) || locks[lock_descriptor].state == LOCK_FREE) {
        restore(ps);
        return SYSERR;
    }

    LOCK_DEBUG_PRINT("Deleting lock with descriptor: %d", lock_descriptor);
    
    struct lock *lock_ptr = &locks[lock_descriptor];
    lock_ptr->state = LOCK_FREE;
    lock_ptr->type = 0;
    lock_ptr->num_holding_processes = 0;
    lock_ptr->max_lock_priority = 0;
    lock_ptr->wait_time = 0;
    
    // Release all processes waiting on this lock
    if (nonempty(lock_ptr->queue_head)) {

        // Get first pops the first entry and returns it
        int pid = getfirst(lock_ptr->queue_head);

        // While we keep popping, set the process' return value to DELETED
        // and make it ready for rescheduling
        while (pid != EMPTY) {
            process_locking_infos[pid].wait_return_value = DELETED;
            process_locking_infos[pid].lock_descriptor = -1;
            process_locking_infos[pid].lock_type = 0;
            process_locking_infos[pid].max_process_priority = 0;
            process_locking_infos[pid].wait_start_time = 0;
            ready(pid, RESCHNO);
            pid = getfirst(lock_ptr->queue_head);
        }

        // Reschedule released processes
        resched();
    }

    lock_ptr->queue_head = lock_ptr->default_queue_head;
    lock_ptr->queue_tail = lock_ptr->default_queue_tail;
    for(int i = 0; i < NPROC; i++) {
        lock_ptr->process_is_holding[i] = FALSE;
    }

    LOCK_DEBUG_PRINT("Lock deleted with descriptor: %d", lock_descriptor);
    
    restore(ps);
    return OK;
} 