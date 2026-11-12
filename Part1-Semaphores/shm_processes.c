/*
 * Project 2: Process Synchronization Using Semaphores
 *
 * Description:
 * This program simulates a shared family bank account.
 * Dear Old Dad and Lovable Mom deposit money, while Poor
 * Students withdraw money from the account.
 *
 * Shared memory allows all processes to access the same
 * BankAccount variable. A POSIX semaphore ensures mutual
 * exclusion and prevents race conditions during updates.
 *
 * Command-line arguments:
 *   1 1 - One Dad and one Student
 *   2 3 - Dad, Mom, and three Students
 *
 * Each process runs continuously until terminated.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>

typedef struct {
    int BankAccount;
    sem_t mutex;
} SharedData;

SharedData *shared;

void dad(void) {
    int localBalance = 0;

    srand(time(NULL) ^ getpid());

    while (1) {
        sleep(rand() % 6);

        printf("Dear Old Dad: Attempting to Check Balance\n");

        if (rand() % 2 == 0) {
            if (localBalance < 100) {
                // Enter critical section to safely access the shared account.
                sem_wait(&shared->mutex);

                localBalance = shared->BankAccount;
                int amount = rand() % 101;

                if (rand() % 2 == 0) {
                    localBalance += amount;
                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n",
                           amount, localBalance);
                } else {
                    printf("Dear old Dad: Doesn't have any money to give\n");
                }

                shared->BankAccount = localBalance;
                sem_post(&shared->mutex);
            } else {
                printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n",
                       localBalance);
            }
        } else {
            printf("Dear Old Dad: Last Checking Balance = $%d\n",
                   localBalance);
        }

        fflush(stdout);
    }
}

void mom(void) {
    int localBalance = 0;

    srand(time(NULL) ^ getpid());

    while (1) {
        sleep(rand() % 11);

        printf("Loveable Mom: Attempting to Check Balance\n");

        if (localBalance <= 100) {
            sem_wait(&shared->mutex);

            localBalance = shared->BankAccount;
            int amount = rand() % 126;

            localBalance += amount;

            printf("Lovable Mom: Deposits $%d / Balance = $%d\n",
                   amount, localBalance);

            shared->BankAccount = localBalance;

            sem_post(&shared->mutex);
        }

        fflush(stdout);
    }
}

void student(int number) {
    int localBalance = 0;

    srand(time(NULL) ^ getpid());

    while (1) {
        sleep(rand() % 6);

        printf("Poor Student: Attempting to Check Balance\n");

        if (rand() % 2 == 0) {
            sem_wait(&shared->mutex);

            localBalance = shared->BankAccount;
            int need = rand() % 51;

            printf("Poor Student needs $%d\n", need);

            if (need <= localBalance) {
                localBalance -= need;

                printf("Poor Student: Withdraws $%d / Balance = $%d\n",
                       need, localBalance);
            } else {
                printf("Poor Student: Not Enough Cash ($%d)\n",
                       localBalance);
            }

            shared->BankAccount = localBalance;

            sem_post(&shared->mutex);
        } else {
            printf("Poor Student: Last Checking Balance = $%d\n",
                   localBalance);
        }

        fflush(stdout);
    }
}

int main(int argc, char *argv[]) {
    int parents = 1;
    int students = 1;

    if (argc == 3) {
        parents = atoi(argv[1]);
        students = atoi(argv[2]);
    } else if (argc != 1) {
        printf("Usage: %s [1|2] [number_of_students]\n", argv[0]);
        return 1;
    }

    if (parents < 1 || parents > 2 || students < 1) {
        printf("Invalid arguments.\n");
        return 1;
    }

    shared = mmap(NULL, sizeof(SharedData),
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (shared == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    shared->BankAccount = 0;

    if (sem_init(&shared->mutex, 1, 1) == -1) {
        perror("sem_init");
        return 1;
    }

    for (int i = 0; i < students; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            student(i + 1);
            _exit(0);
        } else if (pid < 0) {
            perror("fork");
            return 1;
        }
    }

    if (parents == 2) {
        pid_t pid = fork();

        if (pid == 0) {
            mom();
            _exit(0);
        } else if (pid < 0) {
            perror("fork");
            return 1;
        }
    }

    dad();

    return 0;
}
