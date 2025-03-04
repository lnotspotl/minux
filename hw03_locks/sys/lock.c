/* lock.c - lock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <lock.h>
#include <stdio.h>

extern unsigned long ctr1000;  // counter in milliseconds

int lock(int lock_descriptor, int lock_type, int lock_priority)
{
    STATWORD ps;

    // Disable interrupts
    disable(ps);

    // Error checking
    if (lock_isbad(lock_descriptor) || locks[lock_descriptor].state == LOCK_FREE) {
        restore(ps);
        return SYSERR;
    }

    // Check if the lock type is valid
    if (lock_type != LOCK_READ && lock_type != LOCK_WRITE) {
        restore(ps);
        return SYSERR;
    }

    LOCK_DEBUG_PRINT("Locking with descriptor: %d, type: %s, lock priority: %d, process priority: %d, pid: %d (%s)", lock_descriptor, lock_type_to_string(lock_type), lock_priority, getprio(currpid), currpid, proctab[currpid].pname);

    // pid of the currently running process
    int pid = currpid;

    struct lock *lock_ptr = &locks[lock_descriptor];
    struct pentry *process_ptr = &proctab[pid];


    // Case 1: Lock is free, grant lock
    if (lock_ptr->num_holding_processes == 0) {
        lock_ptr->state = LOCK_USED;
        lock_ptr->type = lock_type;
        LOCK_ASSERT(isempty(lock_ptr->queue_head), "Lock queue is not empty");
        lock_ptr->num_holding_processes = 1;

        for(int i = 1; i < NPROC; ++i) {
            LOCK_ASSERT(lock_ptr->process_is_holding[i] == FALSE, "Process is holding lock");
        }
        
        lock_ptr->process_is_holding[pid] = TRUE;
        lock_ptr->max_lock_priority = lock_priority;
        
        LOCK_DEBUG_PRINT("(1) Lock granted to process %d (%s)", pid, proctab[pid].pname);
        restore(ps);
        return OK;
    }


    // Highest priority writer waiting 
    int highest_writer_prio = -1;
    
    // Find the priority of the highest priority writer waiting for this lock
    if (nonempty(lock_ptr->queue_head)) {
        int next_pid = q[lock_ptr->queue_head].qnext;
        while (next_pid < NPROC) {
            if (process_locking_infos[next_pid].lock_type == LOCK_WRITE) {
                if (process_locking_infos[next_pid].max_process_priority > highest_writer_prio) {
                    highest_writer_prio = process_locking_infos[next_pid].max_process_priority;
                }
            }
            next_pid = q[next_pid].qnext;
        }
    }

    LOCK_DEBUG_PRINT("Highest priority writer waiting: %d", highest_writer_prio);

    
    // Case 2: Lock is held for reading and request is for reading
    if (lock_ptr->type == LOCK_READ && lock_type == LOCK_READ) {
        // Check if there's a higher or equal priority writer waiting
        if (highest_writer_prio >= lock_priority) {
            // Enqueue the process and block it
            process_locking_infos[pid].lock_descriptor = lock_descriptor;
            process_locking_infos[pid].lock_type = lock_type;
            process_locking_infos[pid].max_process_priority = lock_priority;
            process_locking_infos[pid].wait_return_value = OK;
            process_locking_infos[pid].wait_start_time = ctr1000;
            
            // Update lock priority if needed
            if (lock_priority > lock_ptr->max_lock_priority) {
                lock_ptr->max_lock_priority = lock_priority;
            }
            
            // Insert into wait queue based on priority
            insert(pid, lock_ptr->queue_head, lock_priority);

            // Apply priority inheritance
            run_priority_inheritance_algo();

            // Block the process
            LOCK_DEBUG_PRINT("(2) Process %d blocked (%s)", pid, proctab[pid].pname);
            process_ptr->pstate = PRWAIT;
            resched();
            LOCK_DEBUG_PRINT("(2) Process %d (%s) unblocked", currpid, proctab[currpid].pname);
            restore(ps);
            return process_locking_infos[currpid].wait_return_value;
        } else {
            // No higher priority writer waiting, grant the lock
            lock_ptr->num_holding_processes++;
            lock_ptr->process_is_holding[pid] = TRUE;
            
            // Update lock priority if needed
            if (lock_priority > lock_ptr->max_lock_priority) {
                lock_ptr->max_lock_priority = lock_priority;
            }
            
            LOCK_DEBUG_PRINT("(3) Lock granted to process %d (%s)", pid, proctab[pid].pname);
            restore(ps);
            return OK;
        }
    }
    
    // Case 3: Lock is held for writing or request is for writing
    // Enqueue the process and block it
    process_locking_infos[pid].lock_descriptor = lock_descriptor;
    process_locking_infos[pid].lock_type = lock_type;
    process_locking_infos[pid].max_process_priority = lock_priority;
    process_locking_infos[pid].wait_return_value = OK;
    process_locking_infos[pid].wait_start_time = ctr1000;
    
    // Update lock priority if needed
    if (lock_priority > lock_ptr->max_lock_priority) {
        lock_ptr->max_lock_priority = lock_priority;
    }
    
    // Insert into wait queue based on priority
    insert(pid, lock_ptr->queue_head, lock_priority);

    // Apply priority inheritance
    run_priority_inheritance_algo();

    LOCK_DEBUG_PRINT("(4) Process %d blocked (%s)", pid, proctab[pid].pname);
    
    // Block the process
    process_ptr->pstate = PRWAIT;
    resched();

    LOCK_DEBUG_PRINT("(4) Process %d unblocked (%s) waiting for lock %d", currpid, proctab[currpid].pname, lock_descriptor);
    
    restore(ps);
    return process_locking_infos[pid].wait_return_value;
}

int isbadlock(int lock_descriptor) {
    return (lock_descriptor < 0 || lock_descriptor >= NUM_LOCKS);
}

void run_priority_inheritance_algo() {
    restore_priority_inheritance();
    run_priority_inheritance();
}

void restore_priority_inheritance() {
    for (int i = 0; i < NPROC; i++) {
        if (proctab[i].pstate == PRFREE) {
            continue;
        }
        chpriol(i, proctab[i].original_priority);
        LOCK_DEBUG_PRINT("Restoring priority inheritance for process %d (%s) to %d", i, proctab[i].pname, proctab[i].original_priority);
    }
    for (int i = 0; i < NLOCKS; i++) {
        if (locks[i].state == LOCK_USED) {
            locks[i].max_lock_priority = -3000;  
            LOCK_DEBUG_PRINT("Restoring priority inheritance for lock %d to %d", i, -3000);
        }
    }


}

void run_priority_inheritance() {
    for(int pid = 0; pid < NPROC; ++pid) {

        // Run this only for running processes
        if (proctab[pid].pstate == PRFREE) {
            continue;
        }

        run_priority_inheritance_for_process(pid);
    }
}
    
void run_priority_inheritance_for_process(int pid) {

    int waiting_for = process_locking_infos[pid].lock_descriptor;
    if (waiting_for == -1) {
        return;
    }

    if (locks[waiting_for].max_lock_priority < getprio(pid)) {
        locks[waiting_for].max_lock_priority = getprio(pid);
        LOCK_DEBUG_PRINT("Running priority inheritance for process %d (%s) to lock %d to %d", pid, proctab[pid].pname, waiting_for, locks[waiting_for].max_lock_priority);
        run_priority_inheritance_for_lock(waiting_for);
    }
}

void run_priority_inheritance_for_lock(int lock_descriptor) {
    for(int i = 0; i < NPROC; ++i) {
        if (locks[lock_descriptor].process_is_holding[i] == TRUE) {
            LOCK_DEBUG_PRINT("Running priority inheritance for lock %d to process %d (%s) to %d (original priority: %d)", lock_descriptor, i, proctab[i].pname, locks[lock_descriptor].max_lock_priority, proctab[i].original_priority);
            chpriol(i, locks[lock_descriptor].max_lock_priority);
            LOCK_DEBUG_PRINT("Running priority inheritance for process %d (%s) to lock %d to %d (original priority: %d)", i, proctab[i].pname, lock_descriptor, locks[lock_descriptor].max_lock_priority, proctab[i].original_priority);
            run_priority_inheritance_for_process(i);
        }
    }
}

int chpriol(int pid, int priority) {
    STATWORD ps;
    disable(ps);
    int old_original_priority = proctab[pid].original_priority;
    int ret = chprio_clean(pid, priority);
    proctab[pid].original_priority = old_original_priority;
    restore(ps);
    return ret;
}