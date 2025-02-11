## Q&A

```
Q: What are the advantages and disadvantages of each of the two scheduling policies? Also, give the advantages and disadvantages of the round robin scheduling policy originally implemented in Xinu.

A: 

Age-based scheduling:
- Advantages:
  - Easy to implement

- Disadvantages:
  - Overhead compared to the XINU scheduler (more book-keeping)

Linux-Based scheduling:
- Advantages:
  - Used in the Linux kernel, so it must be good
  - Seems to schedule processes in proportion to their priorities
  - Feedback-loop: new quantum is calculated based on counter previous epoch
- Disadvantages:
  - Way more difficult to implement, bug prone
```

```
Q: Describe when each of the schedulers run the NULL process.
A: 
- Age-based scheduling: Only when there are no other running or ready processes
- Linux-based scheduling: Only when there are no other running or ready processes
```

```
Q: Give two suggestions how the above mentioned aging based scheduler could be changed to attain better fairness keeping the fundamental point of incrementing the priority to be the same in your new ideas.
A:
- Incorporate the feedback loop from the Linux-based scheduler
- Use different increment value (maybe 2 is not optimal for all cases)
```