/* task1.c - test priority inheritance with readers/writer locks */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <stdio.h>
#include <sem.h>

#define LOW_PRIO    10
#define MED_PRIO    20
#define HIGH_PRIO   30

void lock_proc_low(void);
void lock_proc_med(void);
void lock_proc_high(void);
void test_locks(void);
void test_semaphores(void);
void sem_proc_low(void);
void sem_proc_med(void);
void sem_proc_high(void);

int lock_descriptor;
int semaphore_descriptor;
int main()
{
    kprintf("Priority Inheritance Test\n\n");
    kprintf("Testing implementation with locks\n\n");
    test_locks();

    kprintf("Testing with Semaphores (without priority inheritance):\n");
    test_semaphores();
    
    
    return 0;
}

void test_semaphores()
{
    semaphore_descriptor = screate(1);
    if (semaphore_descriptor == SYSERR) {
        kprintf("Error creating semaphore\n");
        return;
    }

    int low_proc = create((int *)sem_proc_low, 2000, LOW_PRIO, "proc_low", 0, 0);
    int med_proc = create((int *)sem_proc_med, 2000, MED_PRIO, "proc_med", 0, 0);
    int high_proc = create((int *)sem_proc_high, 2000, HIGH_PRIO, "proc_high", 0, 0);

    resume(low_proc);
    sleep(3);
    resume(med_proc);
    sleep(3);
    resume(high_proc);

    sleep(10);
}

void test_locks()
{

    // Create lock
    int lock_descriptor = lcreate();
    if (lock_descriptor == SYSERR) {
        kprintf("Error creating lock\n");
        return;
    }
    
    /* Create processes */
    int low_proc = create((int *)lock_proc_low, 2000, LOW_PRIO, "proc_low", 0, 0);
    int med_proc = create((int *)lock_proc_med, 2000, MED_PRIO, "proc_med", 0, 0);
    int high_proc = create((int *)lock_proc_high, 2000, HIGH_PRIO, "proc_high", 0, 0);
    
    /* Start processes */
    resume(low_proc);
    sleep(3);
    resume(med_proc);
    sleep(3);
    resume(high_proc);

    sleep(10);
}

void lock_proc_low()
{

    kprintf("Low priority process started with priority %d\n", getprio(getpid()));
    int i;
    lock(lock_descriptor, WRITE, HIGH_PRIO);

    for (i = 0; i < 5; i++) {
        kprintf("Low priority process working with priority %d...\n", getprio(getpid()));
        sleep(1);
    }
    
    kprintf("Low priority process completed with priority %d\n", getprio(getpid()));

    release(lock_descriptor);
}

void lock_proc_med()
{
    kprintf("Medium priority process started with priority %d\n", getprio(getpid()));
    int i;
    lock(lock_descriptor, WRITE, MED_PRIO);

    for (i = 0; i < 5; i++) {
        kprintf("Medium priority process working with priority %d...\n", getprio(getpid()));
        sleep(1);
    }
    
    kprintf("Medium priority process completed with priority %d\n", getprio(getpid()));

    release(lock_descriptor);
}

void lock_proc_high()
{
    kprintf("High priority process started with priority %d\n", getprio(getpid()));
    int i;
    lock(lock_descriptor, WRITE, LOW_PRIO);

    for (i = 0; i < 5; i++) {
        kprintf("High priority process working with priority %d...\n", getprio(getpid()));
        sleep(1);
    }
    
    kprintf("High priority process completed with priority %d\n", getprio(getpid()));

    signal(semaphore_descriptor);
}

void sem_proc_low()
{

    kprintf("Low priority process started with priority %d\n", getprio(getpid()));
    int i;
    wait(semaphore_descriptor);

    for (i = 0; i < 5; i++) {
        kprintf("Low priority process working with priority %d...\n", getprio(getpid()));
        sleep(1);
    }
    
    kprintf("Low priority process completed with priority %d\n", getprio(getpid()));

    signal(semaphore_descriptor);
}

void sem_proc_med()
{
    kprintf("Medium priority process started with priority %d\n", getprio(getpid()));
    int i;
    wait(semaphore_descriptor);

    for (i = 0; i < 5; i++) {
        kprintf("Medium priority process working with priority %d...\n", getprio(getpid()));
        sleep(1);
    }
    
    kprintf("Medium priority process completed with priority %d\n", getprio(getpid()));

    signal(semaphore_descriptor);
}

void sem_proc_high()
{
    kprintf("High priority process started with priority %d\n", getprio(getpid()));
    int i;
    wait(semaphore_descriptor);

    for (i = 0; i < 5; i++) {
        kprintf("High priority process working with priority %d...\n", getprio(getpid()));
        sleep(1);
    }
    
    kprintf("High priority process completed with priority %d\n", getprio(getpid()));

    signal(semaphore_descriptor);
}