#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <lock.h>
#include <stdio.h>

extern unsigned long ctr1000;  // counter in milliseconds

int lock_release(int lock_descriptor)
{
    STATWORD ps;
    struct lock *lptr;
    int result = OK;
    int pid;
    int readers_to_wake = 0;
    int writers_to_wake = 0;
    int highest_prio_proc = -1;
    int highest_prio = -1;
    int highest_writer_prio = -1;
    int highest_reader_prio = -1;
    int highest_writer_pid = -1;
    int highest_reader_pid = -1;
    unsigned long longest_wait_time = 0;
    unsigned long writer_wait_time = 0;
    unsigned long reader_wait_time = 0;
    
    disable(ps);
    
    // Check if lock descriptor is valid
    if (lock_isbad(lock_descriptor) || locks[lock_descriptor].state == LOCK_FREE) {
        restore(ps);
        return SYSERR;
    }
    
    lptr = &locks[lock_descriptor];
    pid = currpid;
    
    // Check if the process holds the lock
    if (lptr->process_is_holding[pid] != TRUE) {
        restore(ps);
        return SYSERR;
    }

    LOCK_DEBUG_PRINT("Releasing lock %d with process %d (%s)", lock_descriptor, pid, proctab[pid].pname);
    
    // Release the lock
    lptr->process_is_holding[pid] = FALSE;
    lptr->num_holding_processes--;

    run_priority_inheritance_algo();
    
    // If the lock is still held by other processes, return
    if (lptr->num_holding_processes > 0) {
        LOCK_DEBUG_PRINT("(0) Lock %d is still held by other processes", lock_descriptor);
        restore(ps);
        return OK;
    }
    
    // Lock is now free, check if there are processes waiting
    if (nonempty(lptr->queue_head)) {
        // Find the highest priority process waiting for the lock
        int next_pid = q[lptr->queue_head].qnext;
        highest_prio = -1;
        highest_writer_prio = -1;
        highest_reader_prio = -1;
        longest_wait_time = 0;
        writer_wait_time = 0;
        reader_wait_time = 0;
        
        // Scan the wait queue to find highest priority processes
        while (next_pid < NPROC) {
            int wait_prio = process_locking_infos[next_pid].max_process_priority;
            unsigned long wait_time = ctr1000 - process_locking_infos[next_pid].wait_start_time;
            
            // Update highest priority process
            if (wait_prio > highest_prio || 
                (wait_prio == highest_prio && wait_time > longest_wait_time)) {
                highest_prio = wait_prio;
                highest_prio_proc = next_pid;
                longest_wait_time = wait_time;
            }
            
            // Update highest priority writer
            if (process_locking_infos[next_pid].lock_type == LOCK_WRITE) {
                if (wait_prio > highest_writer_prio || 
                    (wait_prio == highest_writer_prio && wait_time > writer_wait_time)) {
                    highest_writer_prio = wait_prio;
                    highest_writer_pid = next_pid;
                    writer_wait_time = wait_time;
                }
            }
            
            // Update highest priority reader
            if (process_locking_infos[next_pid].lock_type == LOCK_READ) {
                if (wait_prio > highest_reader_prio || 
                    (wait_prio == highest_reader_prio && wait_time > reader_wait_time)) {
                    highest_reader_prio = wait_prio;
                    highest_reader_pid = next_pid;
                    reader_wait_time = wait_time;
                }
            }
            
            next_pid = q[next_pid].qnext;
        }
        
        // Determine which processes to wake up based on priority and wait time
        if (highest_writer_prio > 0) {
            // If there's a writer waiting, prioritize it if:
            // 1. Writer priority > reader priority, OR
            // 2. Writer priority == reader priority AND writer wait time is not more than 0.5s longer than reader
            if (highest_writer_prio > highest_reader_prio || 
                (highest_writer_prio == highest_reader_prio && 
                 (writer_wait_time >= reader_wait_time || 
                  reader_wait_time - writer_wait_time <= 500))) { // 500ms = 0.5s
                writers_to_wake = 1;
            } else {
                readers_to_wake = 1;
            }
        } else {
            // No writers waiting, wake up readers
            readers_to_wake = 1;
        }
        
        // Wake up processes based on decision
        if (writers_to_wake) {
            // Wake up the highest priority writer
            dequeue(highest_writer_pid);
            process_locking_infos[highest_writer_pid].lock_descriptor = -1;
            process_locking_infos[highest_writer_pid].wait_return_value = OK;
            
            // Grant the lock to the writer
            lptr->type = LOCK_WRITE;
            lptr->num_holding_processes = 1;
            lptr->process_is_holding[highest_writer_pid] = TRUE;
            
            // Update process priority after releasing the lock
            LOCK_DEBUG_PRINT("(1) Process %d (%s) released lock %d and is now a writer", highest_writer_pid, proctab[highest_writer_pid].pname, lock_descriptor);
            
            run_priority_inheritance_algo();
            
            // Ready the process
            ready(highest_writer_pid, RESCHYES);
        } else if (readers_to_wake) {
            // Wake up all readers in priority order
            lptr->type = LOCK_READ;
            
            // First, collect all eligible readers
            int reader_pids[NPROC];
            int reader_prios[NPROC];
            int num_readers = 0;
            
            next_pid = q[lptr->queue_head].qnext;
            while (next_pid < NPROC) {
                int curr_pid = next_pid;
                next_pid = q[next_pid].qnext;
                
                if (process_locking_infos[curr_pid].lock_type == LOCK_READ && 
                    (highest_writer_prio <= 0 || process_locking_infos[curr_pid].max_process_priority > highest_writer_prio)) {
                    reader_pids[num_readers] = curr_pid;
                    reader_prios[num_readers] = process_locking_infos[curr_pid].max_process_priority;
                    num_readers++;
                }
            }
            
            // Sort readers by priority (highest first) using a simple bubble sort
            for (int i = 0; i < num_readers - 1; i++) {
                for (int j = 0; j < num_readers - i - 1; j++) {
                    if (reader_prios[j] < reader_prios[j + 1]) {
                        // Swap priorities
                        int temp_prio = reader_prios[j];
                        reader_prios[j] = reader_prios[j + 1];
                        reader_prios[j + 1] = temp_prio;
                        
                        // Swap PIDs
                        int temp_pid = reader_pids[j];
                        reader_pids[j] = reader_pids[j + 1];
                        reader_pids[j + 1] = temp_pid;
                    }
                }
            }
            
            // Wake up readers in priority order
            for (int i = 0; i < num_readers; i++) {
                int curr_pid = reader_pids[i];
                
                // Remove from wait queue
                dequeue(curr_pid);
                process_locking_infos[curr_pid].lock_descriptor = -1;
                process_locking_infos[curr_pid].wait_return_value = OK;
                
                // Grant the lock to the reader
                lptr->num_holding_processes++;
                lptr->process_is_holding[curr_pid] = TRUE;
                
                // Update process priority after releasing the lock
                LOCK_DEBUG_PRINT("(2) Process %d (%s) released lock %d and is now a reader", curr_pid, proctab[curr_pid].pname, lock_descriptor);
                run_priority_inheritance_algo();
                
                // Ready the process
                ready(curr_pid, RESCHYES);
            }
        }
    } else {
        // No processes waiting, mark the lock as free
        lptr->type = 0;
    }
    
    LOCK_DEBUG_PRINT("(3) Process %d (%s) released lock %d and is now free", pid, proctab[pid].pname, lock_descriptor);
    
    run_priority_inheritance_algo();
    
    // Reschedule if necessary
    resched();
    
    restore(ps);
    return result;
}
