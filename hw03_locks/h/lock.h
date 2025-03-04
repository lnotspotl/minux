#ifndef _LOCK_H_
#define _LOCK_H_

#include <kernel.h>
#include <proc.h>

#define LOCK_DISABLE_DEBUG  

#ifdef LOCK_DISABLE_DEBUG
#define LOCK_DEBUG_PRINT(fmt, ...)
#define LOCK_ASSERT(x, message)
#else
#define LOCK_DEBUG_PRINT(fmt, ...) \
        kprintf("\e[0;33m[DEBUG]\e[0m %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, __VA_ARGS__); 

#define LOCK_ASSERT(x, message) if (!(x)) { \
    LOCK_DEBUG_PRINT("%s", message); \
    return SYSERR; \
}
#endif

// Lock states
#define LOCK_FREE    123   // this lock is free
#define LOCK_USED    124   // this lock is used

// Lock types
#define LOCK_READ     57   // lock for reading
#define LOCK_WRITE    58   // lock for writing

// Lock table entry
struct lock {
    char state;                         // state of the lock: LOCK_FREE or LOCK_USED
    char type;                          // type of the lock: LOCK_READ or LOCK_WRITE
    int default_queue_head;             // index of the queue head of processes waiting for the lock
    int default_queue_tail;             // index of the queue tail of processes waiting for the lock
    int queue_head;                     // index of the queue head of processes waiting for the lock
    int queue_tail;                     // index of the queue tail of processes waiting for the lock
    int max_lock_priority;              // maximum priority among all waiting processes
    int num_holding_processes;          // count of processes currently holding the lock
    unsigned long wait_time;            // time when the first process started waiting
    char process_is_holding[NPROC];     // list of processes holding the lock
};

// Number of locks in the system
#define NUM_LOCKS 50

// An array of available locks, size of NUM_LOCKS
extern struct lock locks[];

// Index of the next available lock
extern int next_lock_index;

// Process table extensions for locks
struct process_locking_info {
    int inherited_priority;             // inherited priority of the process
    int lock_descriptor;                // lock descriptor of the lock this process is waiting for
    char lock_type;                     // type of lock the process is waiting for (LOCK_READ or LOCK_WRITE)
    int max_process_priority;           // priority of the process for the lock (highest priority of all processes waiting for the lock)
    unsigned long wait_return_value;    // return value when waiting for the lock
    unsigned long wait_start_time;      // time when the process started waiting
};

// An array of process locks, size of NPROC
extern struct process_locking_info process_locking_infos[];

// Initialize system locks
int lock_init();

// Create a new system lock. Return lock descriptor on success, SYSERR on failure
int lock_create();

// Delete a new system lock. Return OK on success, SYSERR on failure
int lock_delete(int lockdescriptor);

// Lock a resource. Return OK on success, SYSERR on failure
// lock_descriptor - descriptor of the lock to lock
// type - type of lock - READ or WRITE
// priority - priority of the process for the lock
int lock(int lock_descriptor, int type, int priority);

// Release a single lock defined by the lock_descriptor
int lock_release(int lock_descriptor);

// Release all locks
int lock_releaseall(int numlocks, int args, ...);

// Check if the specific lock descriptor is valid
int lock_isbad(int lock_descriptor);


void run_priority_inheritance_algo();
void restore_priority_inheritance();
void run_priority_inheritance();
void run_priority_inheritance_for_process(int pid);
void run_priority_inheritance_for_lock(int lock_descriptor);
int chpriol(int pid, int priority);


#define lock_type_to_string(type) (type == LOCK_READ ? "READ" : (type == LOCK_WRITE ? "WRITE" : "UNKNOWN"))

#define linit lock_init
#define lcreate lock_create
#define ldelete lock_delete
#define release lock_release
#define releaseall lock_releaseall
#define isbadlock lock_isbad

// Complying with the requirements
#define READ LOCK_READ 
#define WRITE LOCK_WRITE

#define FREE LOCK_FREE
#define USED LOCK_USED

#define NLOCKS NUM_LOCKS

#endif /* _LOCK_H_ */ 