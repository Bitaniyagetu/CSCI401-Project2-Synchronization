
#include "BENSCHILLIBOWL.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>

/*
 * Project 2 - Part 2: Thread Synchronization
 * Restaurant order management using mutexes and condition variables.
 *
 * Customers produce orders and cooks consume them.
 * The mutex protects shared restaurant data from race conditions.
 */

bool IsEmpty(BENSCHILLIBOWL* bcb);
bool IsFull(BENSCHILLIBOWL* bcb);
void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = {
    "BensChilli",
    "BensHalfSmoke",
    "BensHotDog",
    "BensChilliCheeseFries",
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings"
};

int BENSCHILLIBOWLMenuLength = 10;

/* Select a random menu item. */
MenuItem PickRandomMenuItem() {
    static pthread_mutex_t menu_mutex = PTHREAD_MUTEX_INITIALIZER;
    static unsigned int seed = 12345;

    pthread_mutex_lock(&menu_mutex);
    seed = seed * 1103515245u + 12345u;
    int index = (seed >> 16) % BENSCHILLIBOWLMenuLength;
    pthread_mutex_unlock(&menu_mutex);

    return BENSCHILLIBOWLMenu[index];
}

/* Allocate and initialize the restaurant and synchronization objects. */
BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
    BENSCHILLIBOWL* bcb = malloc(sizeof(BENSCHILLIBOWL));

    if (bcb == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    bcb->orders = NULL;
    bcb->current_size = 0;
    bcb->max_size = max_size;
    bcb->next_order_number = 1;
    bcb->orders_handled = 0;
    bcb->expected_num_orders = expected_num_orders;

    pthread_mutex_init(&bcb->mutex, NULL);
    pthread_cond_init(&bcb->can_add_orders, NULL);
    pthread_cond_init(&bcb->can_get_orders, NULL);

    printf("Restaurant is open!\n");
    return bcb;
}

/* Verify completion and release the restaurant resources. */
void CloseRestaurant(BENSCHILLIBOWL* bcb) {
    assert(bcb != NULL);

    assert(bcb->orders_handled == bcb->expected_num_orders);
    assert(bcb->current_size == 0);

    pthread_mutex_destroy(&bcb->mutex);
    pthread_cond_destroy(&bcb->can_add_orders);
    pthread_cond_destroy(&bcb->can_get_orders);

    free(bcb);

    printf("Restaurant is closed!\n");
}

/* Add an order safely to the back of the queue. */
int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
    pthread_mutex_lock(&bcb->mutex);

    while (IsFull(bcb)) {
        pthread_cond_wait(&bcb->can_add_orders, &bcb->mutex);
    }

    order->order_number = bcb->next_order_number++;
    order->next = NULL;

    AddOrderToBack(&bcb->orders, order);
    bcb->current_size++;

    int order_number = order->order_number;

    printf("Customer #%d placed order #%d: %s\n",
           order->customer_id, order_number, order->menu_item);

    pthread_cond_signal(&bcb->can_get_orders);
    pthread_mutex_unlock(&bcb->mutex);

    return order_number;
}

/* Remove an order safely, waiting if the queue is empty. */
Order* GetOrder(BENSCHILLIBOWL* bcb) {
    pthread_mutex_lock(&bcb->mutex);

    while (IsEmpty(bcb) &&
           bcb->next_order_number <= bcb->expected_num_orders) {
        pthread_cond_wait(&bcb->can_get_orders, &bcb->mutex);
    }

    if (IsEmpty(bcb)) {
        pthread_cond_broadcast(&bcb->can_get_orders);
        pthread_mutex_unlock(&bcb->mutex);
        return NULL;
    }

    Order* order = bcb->orders;
    bcb->orders = order->next;
    order->next = NULL;

    bcb->current_size--;
    bcb->orders_handled++;

    pthread_cond_signal(&bcb->can_add_orders);

    if (bcb->next_order_number > bcb->expected_num_orders) {
        pthread_cond_broadcast(&bcb->can_get_orders);
    }

    pthread_mutex_unlock(&bcb->mutex);

    return order;
}

/* Check whether the order queue is empty. */
bool IsEmpty(BENSCHILLIBOWL* bcb) {
    return bcb->current_size == 0;
}

/* Check whether the order queue has reached capacity. */
bool IsFull(BENSCHILLIBOWL* bcb) {
    return bcb->current_size >= bcb->max_size;
}

/* Append an order to the end of the linked list. */
void AddOrderToBack(Order **orders, Order *order) {
    order->next = NULL;

    if (*orders == NULL) {
        *orders = order;
        return;
    }

    Order* current = *orders;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = order;
}
