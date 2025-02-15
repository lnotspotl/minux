/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{
	kprintf("\n\nHello World, Xinu lives\n\n");
	kprintf("sizeof(int): %d <-- Should be 4\n", sizeof(int));
	kprintf("sizeof(int): %d <-- Should be 4\n", sizeof(long));

	printsegaddress();

    syscallsummary_start();        

	printtos();
	kprintf("Successfully printed stack trace\n");

	printprocstks(2);

	kprintf("Sleeping for 2 seconds\n");
	sleep(1);
	sleep10(1);
	sleep10(1);
	sleep10(1);
	sleep10(1);
	kprintf("Finished sleeping\n");
    syscallsummary_stop();
    printsyscallsummary();

    int a = 1;
    int is_little_endian = (a & 0xFF) == 1; // Stores lowest bits on the left
    int is_big_endian = (a & 0xFF) == 0; // Stores highest bits on the left

    if (is_little_endian) {
        printf("Little endian\n");
    } else if (is_big_endian) {
        printf("Big endian\n");
    }
    
	return 0;
}
