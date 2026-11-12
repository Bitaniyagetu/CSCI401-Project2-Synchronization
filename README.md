
# CSCI 401 - Project 2: Synchronization

**University:** Howard University  
**Course:** Operating Systems (CSCI 401)  
**Student:** Bitaniya Getu  
**Language:** C

## Project Overview

This project explores synchronization techniques in operating systems through two programming assignments.

### Part 1: Process Synchronization Using Semaphores

Simulates a shared family bank account involving Dear Old Dad, Lovable Mom, and multiple Poor Students.

- Uses shared memory and POSIX semaphores.
- Prevents race conditions during deposits and withdrawals.
- Supports multiple processes using fork().
- Includes the extra-credit implementation.

**Autograder:** 100/100 + 25/25 extra credit.

### Part 2: Thread Synchronization Using Condition Variables

Simulates customers and cooks at Ben's Chili Bowl restaurant.

- Uses POSIX threads (pthreads).
- Implements mutexes and condition variables.
- Uses a synchronized order queue.
- Demonstrates the producer-consumer problem.
- Processes 270 orders using 90 customers and 10 cooks.

**Autograder:** 100/100.

## Compilation and Execution

### Part 1

```bash
gcc -pthread -o shm_proc shm_processes.c
./shm_proc 2 3
```

### Part 2

```bash
make
./main
```

## Acknowledgments

ChatGPT (OpenAI) was used for implementation guidance, code development assistance, explanations of synchronization concepts, debugging, and documentation. The programs were compiled and tested in Codio.

