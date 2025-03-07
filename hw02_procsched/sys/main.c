#include <conf.h>
#include <kernel.h>
#include <q.h>
#include <stdio.h>

#define LOOP 50

int prA, prB, prC;
int proc_a(char), proc_b(char), proc_c(char);
int proc(char);
volatile int a_cnt = 0;
volatile int b_cnt = 0;
volatile int c_cnt = 0;
volatile int s = 0;

int main()
{
	int i;
	int count = 0;
	char buf[8];

	srand(1234);

	kprintf("Please Input:\n");
	while ((i = read(CONSOLE, buf, sizeof(buf))) < 1)
		;
	buf[i] = 0;
	s = atoi(buf);
	kprintf("Get %d\n", s);

	// RANDOMSCHED
	if (s < 2)
	{
		setschedclass(AGESCHED);
		prA = create(proc_a, 2000, 1, "proc A", 1, 'A');
		prB = create(proc_b, 2000, 2, "proc B", 1, 'B');
		prC = create(proc_c, 2000, 3, "proc C", 1, 'C');
		resume(prA);
		resume(prB);
		resume(prC);
		sleep(10);
		kill(prA);
		kill(prB);
		kill(prC);

		kprintf("\nTest Result: A = %d, B = %d, C = %d\n", a_cnt, b_cnt, c_cnt);
	}
	// LINUXSCHED
	else
	{
		setschedclass(LINUXSCHED);
		prA = create(proc, 2000, 5, "proc A", 1, 'A');
		prB = create(proc, 2000, 50, "proc B", 1, 'B');
		prC = create(proc, 2000, 90, "proc C", 1, 'C');
		resume(prA);
		resume(prC);
		resume(prB);

		while (count++ < LOOP)
		{
			kprintf("M");
			for (i = 0; i < 1000000; i++)
				;
		}
		kprintf("Done\n");
		kprintf("\n");
	}
	return 0;
}

int proc_a(char c)
{
	int i;
	kprintf("Start... %c\n", c);
	b_cnt = 0;
	c_cnt = 0;

	while (1)
	{
		for (i = 0; i < 10000; i++)
			;
		a_cnt++;
	}
	return 0;
}

int proc_b(char c)
{
	int i;
	kprintf("Start... %c\n", c);
	a_cnt = 0;
	c_cnt = 0;

	while (1)
	{
		for (i = 0; i < 10000; i++)
			;
		b_cnt++;
	}
	return 0;
}

int proc_c(char c)
{
	int i;
	kprintf("Start... %c\n", c);
	a_cnt = 0;
	b_cnt = 0;

	while (1)
	{
		for (i = 0; i < 10000; i++)
			;
		c_cnt++;
	}
	return 0;
}

int proc(char c)
{
	int i;
	int count = 0;

	while (count++ < LOOP)
	{
		kprintf("%c", c);
		for (i = 0; i < 1000000; i++)
			;
	}
	return 0;
}