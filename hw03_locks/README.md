Lab2Answers.txt - CSC 501 PA2: Readers/Writer Locks with Priority Inheritance

1.1 Alternative approach to address priority inversion:
- Save the state of the process when it acquires the lock
- If a higher priority process is blocked, take the lock from the lower priority process and give it to the higher priority process
- When doing that, restore the state of the lower priority process (downside if there are side-effects in the process whose state is being restored)

1.2

```
Priority Inheritance Test

Testing implementation with locks

Low priority process started with priority 10
Low priority process working with priority 10...
Low priority process working with priority 10...
Low priority process working with priority 10...
Medium priority process started with priority 20
Low priority process working with priority 20...
Low priority process working with priority 20...
Low priority process completed with priority 20
Medium priority process working with priority 20...
Medium priority process working with priority 20...
High priority process started with priority 30
Medium priority process working with priority 30...
Medium priority process working with priority 30...
Medium priority process working with priority 30...
Medium priority process completed with priority 30
High priority process working with priority 30...
High priority process working with priority 30...
High priority process working with priority 30...
High priority process working with priority 30...
High priority process working with priority 30...
High priority process completed with priority 30


Testing with Semaphores (without priority inheritance):
Low priority process started with priority 10
Low priority process working with priority 10...
Low priority process working with priority 10...
Low priority process working with priority 10...
Medium priority process started with priority 20
Low priority process working with priority 10...
Low priority process working with priority 10...
Low priority process completed with priority 10
Medium priority process working with priority 20...
Medium priority process working with priority 20...
High priority process started with priority 30
Medium priority process working with priority 20...
Medium priority process working with priority 20...
Medium priority process working with priority 20...
Medium priority process completed with priority 20
High priority process working with priority 30...
High priority process working with priority 30...
High priority process working with priority 30...
High priority process working with priority 30...
High priority process working with priority 30...
High priority process completed with priority 30


All user processes have completed.


```

As you can see, there is a fundamental difference between my lock implementation and XINU semaphores.
While my lock implementation performs priority inheritance, XINU semaphores do not. Therefore, 
my implementation is better.


2. Potential issues with the following implementation:
```

semaphore resource = 1;
semaphore rmutex = 1;
readcount = 0;
MAX_STRING = 1000;
char buffer[MAX_STRING];

// Function for readers
reader() {
    wait(rmutex);
    readcount++;
    if (readcount == 1)
        wait(resource);
    signal(rmutex);
    char bufstr[MAX_STRING];
    bufstr = read(buffer); // Read from the buffer
    wait(rmutex);
    readcount--;
    if (readcount == 0)
        signal(resource);
    signal(rmutex);
}


// Function for writers
writer() {
    wait(resource);
    char bufstr[MAX_STRING];
    bufstr = pid_to_str(); // Get the string representation of its pid
    write(bufstr, buffer); // Clear the buffer then write to the buffer
    update_global_now();
    signal(resource);
}
```

Assuming wait and signal are implemented atomically, and wait decrements the semaphore by 1 while signal increments it by 1,
these are the potential issues:

- if we call reader() and then writer() and then continue calling reader() really fast,
 then the reader() will continue incrementing readcount indefinitely, causing unwanted reads into the buffer. Remember, we are 
 only waiting for the writer only if readcount is equal to 1, so if it becomes 2, 3, 4, etc, the reader() will not wait for the writer()

