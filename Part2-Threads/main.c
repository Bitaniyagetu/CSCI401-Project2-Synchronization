
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "BENSCHILLIBOWL.h"

/*
 * Project 2 - Part 2: Thread Synchronization
 *
 * Customers produce orders and cooks consume them.
 * Mutexes and condition variables synchronize access
 * to the shared restaurant order queue.
 *
 * Assistance: ChatGPT was used for implementation
 * guidance, explanations, and debugging.
 */

#define BENSCHILLIBOWL_SIZE 100
#define NUM_CUSTOMERS 90
#define NUM_COOKS 10
#define ORDERS_PER_CUSTOMER 3
#define EXPECTED_NUM_ORDERS (NUM_CUSTOMERS * ORDERS_PER_CUSTOMER)

/* Shared restaurant used by all threads. */
BENSCHILLIBOWL *bcb;

/* Each customer creates and submits three orders. */
void* BENSCHILLIBOWLCustomer(void* tid) {
    int customer_id = (int)(long)tid;

    for (int i = 0; i < ORDERS_PER_CUSTOMER; i++) {
        Order* order = malloc(sizeof(Order));

        if (order == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        order->customer_id = customer_id;
        order->menu_item = PickRandomMenuItem();
        order->order_number = 0;
        order->next = NULL;

        AddOrder(bcb, order);
    }

    return NULL;
}

/* Each cook takes orders until no more remain. */
void* BENSCHILLIBOWLCook(void* tid) {
    int cook_id = (int)(long)tid;
    int orders_fulfilled = 0;

    while (1) {
        Order* order = GetOrder(bcb);

        if (order == NULL) {
            break;
        }

        printf("Cook #%d fulfilled order #%d: %s\n",
               cook_id,
               order->order_number,
               order->menu_item);

        orders_fulfilled++;

        /* Release memory after fulfilling the order. */
        free(order);
    }

    printf("Cook #%d fulfilled %d orders\n",
           cook_id, orders_fulfilled);

    return NULL;
}

/* Open restaurant, start threads, wait, and clean up. */
int main(void) {
    pthread_t customers[NUM_CUSTOMERS];
    pthread_t cooks[NUM_COOKS];

    bcb = OpenRestaurant(
        BENSCHILLIBOWL_SIZE,
        EXPECTED_NUM_ORDERS
    );

    /* Start cook threads. */
    for (long i = 0; i < NUM_COOKS; i++) {
        if (pthread_create(&cooks[i], NULL,
                           BENSCHILLIBOWLCook,
                           (void*)(i + 1)) != 0) {
            fprintf(stderr, "Failed to create cook\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Start customer threads. */
    for (long i = 0; i < NUM_CUSTOMERS; i++) {
        if (pthread_create(&customers[i], NULL,
                           BENSCHILLIBOWLCustomer,
                           (void*)(i + 1)) != 0) {
            fprintf(stderr, "Failed to create customer\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Wait until every customer has placed their orders. */
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customers[i], NULL);
    }

    /* Wait until all cooks finish processing orders. */
    for (int i = 0; i < NUM_COOKS; i++) {
        pthread_join(cooks[i], NULL);
    }

    /* Verify completion and release restaurant resources. */
    CloseRestaurant(bcb);

    return 0;
}
