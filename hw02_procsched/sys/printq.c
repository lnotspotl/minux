#include <kernel.h>
#include <q.h>
#include <stdio.h>

LOCAL int counter = 0;

int printq(int head, int tail) {
  int current = q[head].qnext;
  if(current == tail) {
    return (OK);
  }
  kprintf("----------- QUEUE -------------\n");
  while (current != tail) {
    kprintf("pid: %d, key: %d\n", current, q[current].qkey);
    current = q[current].qnext;
  }
  kprintf("--------------------------------\n");
  return (OK);
}

int printq_every_n(int head, int tail, int n) {
    if (counter % n == 0) {
        printq(head, tail);
    }
    counter++;
    return (OK);
}

