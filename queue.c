#include <stdio.h>
#include "queue.h"

// Define the global queue variables here
customer_t *business_head = NULL;
customer_t *business_tail = NULL;
int business_queue_len = 0;

customer_t *economy_head = NULL;
customer_t *economy_tail = NULL;
int economy_queue_len = 0;

void enqueue_customer(customer_t *customer) {
    customer->next = NULL;

    if (customer->class_type == 1) {
        // Business class
        if (business_tail == NULL) {
            business_head = business_tail = customer;
        } else {
            business_tail->next = customer;
            business_tail = customer;
        }
        business_queue_len++;
        // Exact formatting match: queue ID 1
        printf("A customer enters a queue: the queue ID 1, and length of the queue %2d. \n", business_queue_len);
    } else {
        // Economy class
        if (economy_tail == NULL) {
            economy_head = economy_tail = customer;
        } else {
            economy_tail->next = customer;
            economy_tail = customer;
        }
        economy_queue_len++;
        // Exact formatting match: queue ID 0
        printf("A customer enters a queue: the queue ID 0, and length of the queue %2d. \n", economy_queue_len);
    }
}

customer_t* dequeue_customer() {
    customer_t *customer = NULL;

    if (business_head != NULL) {
        customer = business_head;
        business_head = business_head->next;
        if (business_head == NULL) {
            business_tail = NULL;
        }
        business_queue_len--;
    } else if (economy_head != NULL) {
        customer = economy_head;
        economy_head = economy_head->next;
        if (economy_head == NULL) {
            economy_tail = NULL;
        }
        economy_queue_len--;
    }
    return customer;
}