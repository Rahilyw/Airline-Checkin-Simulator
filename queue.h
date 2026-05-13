#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

// Customer structure definition
typedef struct customer {
    int id;
    int class_type;        // 1 = business, 0 = economy
    int arrival_time;      // in 1/10 seconds
    int service_time;      // in 1/10 seconds

    double queue_enter_time;
    double service_start_time;

    int served_by_clerk_id;  // clerk that serves this customer
    int is_served;           // set to 1 when a clerk selects this customer

    pthread_cond_t cond;     // per-customer condition variable
    struct customer *next;   // for linked list in queues
} customer_t;

// Use 'extern' to tell the compiler these variables exist in another file (queue.c)
extern customer_t *business_head;
extern customer_t *business_tail;
extern int business_queue_len;

extern customer_t *economy_head;
extern customer_t *economy_tail;
extern int economy_queue_len;

// Function prototypes
void enqueue_customer(customer_t *customer);
customer_t* dequeue_customer();

#endif // QUEUE_H