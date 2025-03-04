#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

struct lock locks[NUM_LOCKS];
struct process_locking_info process_locking_infos[NPROC];
int next_lock_index;

int lock_init()
{
    STATWORD ps;
    register struct lock *lock_ptr;
    
    // Disable interrupts
    disable(ps);

    // Initialize the next lock index
    next_lock_index = 0;
    
    // Initialize all lock table entries
    for (int i = 0; i < NUM_LOCKS; i++) {
        lock_ptr = &locks[i];
        lock_ptr->state = LOCK_FREE;
        lock_ptr->type = 0;
        lock_ptr->num_holding_processes = 0;
        lock_ptr->max_lock_priority = 0;
        lock_ptr->wait_time = 0;
        
        // Initially, the lock is not held by any process
        for (int j = 0; j < NPROC; j++) {
            lock_ptr->process_is_holding[j] = FALSE;
        }
        
        // Initially, no processes are waiting for the lock
        lock_ptr->queue_head = newqueue();
        lock_ptr->queue_tail = lock_ptr->queue_head + 1;
        lock_ptr->default_queue_head = lock_ptr->queue_head;
        lock_ptr->default_queue_tail = lock_ptr->queue_tail;
    }
    
    // Initialize process lock information
    for (int i = 0; i < NPROC; i++) {
        process_locking_infos[i].inherited_priority = 0;
        process_locking_infos[i].lock_descriptor = -1;
        process_locking_infos[i].lock_type = 0;
        process_locking_infos[i].max_process_priority = 0;
        process_locking_infos[i].wait_return_value = OK;
        process_locking_infos[i].wait_start_time = 0;
    }
    
    // Restore interrupts
    restore(ps);
    return OK;
} 