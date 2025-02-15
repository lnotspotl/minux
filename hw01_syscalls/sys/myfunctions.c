#ifndef MYFUNCTIONS_H
#define MYFUNCTIONS_H

#include <kernel.h>
#include <proc.h>

extern char etext, edata;  // Segment boundary markers from linker

void printsegaddress() {

	// Based on Operating System Design: The Xinu Approach, Figure 3.6
	// https://github.com/Kikou1998/textbook/blob/master/Operating%20System%20Design%20The%20Xinu%20Approach.pdf
    
    // Etext segment
    size_t *current_etext_ptr = (size_t *)&etext;
    size_t current_etext_data = *current_etext_ptr;

    size_t *preceding_etext_ptr = current_etext_ptr - 1;
    size_t preceding_etext_data = *preceding_etext_ptr;

    size_t *following_etext_ptr = current_etext_ptr + 1;
    size_t following_etext_data = *following_etext_ptr;

    // Edata segment
    size_t *current_edata_ptr = (size_t *)&edata;
    size_t current_edata_data = *current_edata_ptr;

    size_t *preceding_edata_ptr = current_edata_ptr - 1;
    size_t preceding_edata_data = *preceding_edata_ptr;

    size_t *following_edata_ptr = current_edata_ptr + 1;
    size_t following_edata_data = *following_edata_ptr;

    // Ebss segment
    size_t *current_ebss_ptr = (size_t *)end;
    size_t current_ebss_data = *current_ebss_ptr;

    size_t *preceding_ebss_ptr = current_ebss_ptr - 1;
    size_t preceding_ebss_data = *preceding_ebss_ptr;

    size_t *following_ebss_ptr = current_ebss_ptr + 1;
    size_t following_ebss_data = *following_ebss_ptr;

    // Print to console
    kprintf("\nCurrent: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x\n",
            current_etext_ptr, current_etext_data,
            current_edata_ptr, current_edata_data,
            current_ebss_ptr, current_ebss_data);

    kprintf("Preceding: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x\n",
            preceding_etext_ptr, preceding_etext_data,
            preceding_edata_ptr, preceding_edata_data,
            preceding_ebss_ptr, preceding_ebss_data);

    kprintf("After: etext[0x%08x]=0x%08x, edata[0x%08x]=0x%08x, ebss[0x%08x]=0x%08x\n",
            following_etext_ptr, following_etext_data,
            following_edata_ptr, following_edata_data,
            following_ebss_ptr, following_ebss_data);
}

void printtos() {

    int i;

    // Stack pointer after function call
	int *sp_after;
    asm("movl %%esp, %0" : "=r" (sp_after)); // esp contains the stack pointer (Figure 3.2)

    // Stack pointer before function call
    int *sp_before;
    asm("movl %%ebp, %0" : "=r" (sp_before)); 
    sp_before = sp_before + 1;  // Points to caller's frame # (Figure 3.4), return address

    // Data located at the stack pointer
    int sp_after_data = *sp_after;

    // Data located at the stack pointer before function call
    int sp_before_data = *sp_before;

    // Collect below stack data
    const int STACK_DEPTH = 4;
    int below_stack_data[STACK_DEPTH];
    int below_stack_pointers[STACK_DEPTH];
    for (i = 0; i < STACK_DEPTH; i++) {
        below_stack_data[i] = *(sp_after - i);
        below_stack_pointers[i] = (int *)(sp_after - i);
    }

    // Print to console
    kprintf("\nBefore[0x%08x]: 0x%08x\n", sp_before, sp_before_data);
    kprintf("After [0x%08x]: 0x%08x\n", sp_after, sp_after_data);

    for (i = 0; i < STACK_DEPTH; i++) {
        kprintf("    element[0x%08x]: 0x%08x\n", 
                below_stack_pointers[i],
                below_stack_data[i]);
    }
}

void printprocstks(int priority) {
    int pid;
    for (pid = 0; pid < NPROC; pid++) {
        struct pentry *proc = &proctab[pid];
        
        // Skip if process doesn't exist or priority is too low
        if (proc->pstate == PRFREE || proc->pprio <= priority) {
            continue;
        }

        // If current process, load esp into sp, else use saved stack pointer
        size_t *sp;
        if (pid == currpid) {
            asm("movl %%esp,%0" : "=r"(sp));
        } else {
            sp = (size_t *)proc->pesp;
        }
        
        // Calculate stack limit (base - size)
        size_t *stklim = (size_t *)((char *)proc->pbase - proc->pstklen);
        
        kprintf("Process [%s]\n", proc->pname);
        kprintf("    pid: %d\n", pid);
        kprintf("    priority: %d\n", proc->pprio); 
        kprintf("    base: 0x%08x\n", proc->pbase);
        kprintf("    limit: 0x%08x\n", stklim);
        kprintf("    len: %d\n", proc->pstklen);
        kprintf("    pointer: 0x%08x\n\n", sp);
    }
}

#endif