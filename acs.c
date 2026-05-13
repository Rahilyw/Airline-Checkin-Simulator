#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include "queue.h" 

#define NUM_CLERKS 5 
#define MAX_CUSTOMERS 1000

// CSC 360 - Project 2: Airline Check-in System
// Author: Rahil Wijeyesekera
// Description: This program simulates an airline check-in system with 5 clerks and two queues (business and economy).

/********** Globals **********/
struct timeval simulation_start;     

// Customer array and count
customer_t customers[MAX_CUSTOMERS];
int num_customers = 0;

// Synchronization
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t customer_available = PTHREAD_COND_INITIALIZER;

// Condition variables for clerks to wait while customers self-serve
pthread_cond_t clerk_cond[NUM_CLERKS + 1];

// Statistics
double total_waiting_time = 0.0;
double total_business_waiting_time = 0.0;
double total_economy_waiting_time = 0.0;
int count_all = 0;
int count_business = 0;
int count_economy = 0;
int customers_finished = 0;          

// Thread arrays
pthread_t clerks_threads[NUM_CLERKS];
pthread_t customers_threads[MAX_CUSTOMERS];


/********** Utility: current simulation time **********/
double getCurrentSimulationTime() {
    struct timeval cur_time;
    gettimeofday(&cur_time, NULL);

    double init_secs = simulation_start.tv_sec +
                       (double)simulation_start.tv_usec / 1000000.0;
    double cur_secs = cur_time.tv_sec +
                      (double)cur_time.tv_usec / 1000000.0;

    return cur_secs - init_secs;
}


/***************** Customer thread *******************/
void* customer_thread_func(void *arg) {
    customer_t *cust = (customer_t *)arg;

    // Simulate arrival time (arrival_time * 0.1s)
    if (usleep(cust->arrival_time * 100000) != 0) {
        perror("usleep failed");
    }

    printf("A customer arrives: customer ID %2d. \n", cust->id);

    pthread_mutex_lock(&queue_mutex);

    cust->is_served = 0;
    cust->queue_enter_time = getCurrentSimulationTime();

    enqueue_customer(cust);

    // Signal that a customer is available to wake sleeping clerks
    pthread_cond_signal(&customer_available);

    // Wait until a clerk chooses this customer
    while (!cust->is_served) {
        pthread_cond_wait(&cust->cond, &queue_mutex);
    }

    // Clerk picked this customer
    cust->service_start_time = getCurrentSimulationTime();
    double waiting_time = cust->service_start_time - cust->queue_enter_time;
    int my_clerk = cust->served_by_clerk_id;

    printf("A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n",
           cust->service_start_time, cust->id, my_clerk);

    // Unlock BEFORE doing the service sleep!
    pthread_mutex_unlock(&queue_mutex);

    // Simulate self-serving service time
    if (usleep(cust->service_time * 100000) != 0) {
        perror("usleep failed");
    }

    double end_time = getCurrentSimulationTime();
    printf("A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n",
           end_time, cust->id, my_clerk);

    // Safely update statistics
    pthread_mutex_lock(&stats_mutex);
    total_waiting_time += waiting_time;
    count_all++;
    if (cust->class_type == 1) {
        total_business_waiting_time += waiting_time;
        count_business++;
    } else {
        total_economy_waiting_time += waiting_time;
        count_economy++;
    }
    pthread_mutex_unlock(&stats_mutex);

    // Notify the clerk that we are done so they can serve others
    pthread_mutex_lock(&queue_mutex);
    customers_finished++;
    pthread_cond_signal(&clerk_cond[my_clerk]);
    pthread_mutex_unlock(&queue_mutex);

    return NULL;
}



/********** Clerk thread ***********************/
void* clerk_thread_func(void *arg) {
    int clerk_id = (int)(long)arg;   // 1..NUM_CLERKS

    while (1) {
        pthread_mutex_lock(&queue_mutex);

        // Wait while both queues are empty and not all customers are finished
        while (business_queue_len == 0 && economy_queue_len == 0 &&
               customers_finished < num_customers) {
            pthread_cond_wait(&customer_available, &queue_mutex);
        }

        // If all customers have been served and queues are empty, exit
        if (customers_finished == num_customers &&
            business_queue_len == 0 && economy_queue_len == 0) {
            pthread_mutex_unlock(&queue_mutex);
            break;
        }

        customer_t *customer = dequeue_customer();

        if (customer != NULL) {
            // Mark that this clerk is serving the customer
            customer->served_by_clerk_id = clerk_id;
            customer->is_served = 1;

            // Notify the customer that they have been chosen
            pthread_cond_signal(&customer->cond);

            // Wait for the customer thread to finish its usleep service time
            pthread_cond_wait(&clerk_cond[clerk_id], &queue_mutex);
        } 
        
        pthread_mutex_unlock(&queue_mutex);
    }

    return NULL;
}



/********** Input parsing **********/
int read_input(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    if (fscanf(fp, "%d", &num_customers) != 1) {
        fprintf(stderr, "Error reading number of customers\n");
        fclose(fp);
        return -1;
    }

    if (num_customers < 0 || num_customers > MAX_CUSTOMERS) {
        fprintf(stderr, "Invalid number of customers (max %d)\n", MAX_CUSTOMERS);
        fclose(fp);
        return -1;
    }

    // Read the file. Included \n to cleanly consume end-of-line characters
    for (int i = 0; i < num_customers; i++) {
        int id, class_type, arrival_time, service_time;
        if (fscanf(fp, "%d:%d,%d,%d\n",
                   &id, &class_type, &arrival_time, &service_time) != 4) {
            fprintf(stderr, "Error reading customer data for customer %d\n", i);
            fclose(fp);
            return -1;
        }

        customers[i].id = id;
        customers[i].class_type = class_type;
        customers[i].arrival_time = arrival_time;
        customers[i].service_time = service_time;
        customers[i].queue_enter_time = 0.0;
        customers[i].service_start_time = 0.0;
        customers[i].served_by_clerk_id = 0;
        customers[i].is_served = 0;
        customers[i].next = NULL;

        if (pthread_cond_init(&customers[i].cond, NULL) != 0) {
            fprintf(stderr, "Error initializing condition variable for customer %d\n", i);
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);
    return 0;
}

/********** Main **********/
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s customers.txt\n", argv[0]);
        exit(1);
    }

    if (read_input(argv[1]) != 0) {
        fprintf(stderr, "Error reading input file\n");
        exit(1);
    }

    // Initialize clerk condition variables
    for (int i = 1; i <= NUM_CLERKS; i++) {
        if (pthread_cond_init(&clerk_cond[i], NULL) != 0) {
            fprintf(stderr, "Failed to initialize clerk condition variable.\n");
            exit(1);
        }
    }

    gettimeofday(&simulation_start, NULL);

    // Create clerk threads
    for (int i = 0; i < NUM_CLERKS; i++) {
        long clerk_id = (long)(i + 1);
        if (pthread_create(&clerks_threads[i], NULL,
                           clerk_thread_func, (void*)clerk_id) != 0) {
            fprintf(stderr, "Error creating clerk thread %d\n", i + 1);
            exit(1);
        }
    }

    // Create customer threads
    for (int i = 0; i < num_customers; i++) {
        if (pthread_create(&customers_threads[i], NULL,
                           customer_thread_func, (void*)&customers[i]) != 0) {
            fprintf(stderr, "Error creating customer thread %d\n", i);
            exit(1);
        }
    }

    // Join customer threads with return value checks
    for (int i = 0; i < num_customers; i++) {
        if (pthread_join(customers_threads[i], NULL) != 0) {
            fprintf(stderr, "Failed to join customer thread %d\n", i);
        }
    }

    // Wake clerks in case they are still waiting and queues are empty
    pthread_mutex_lock(&queue_mutex);
    pthread_cond_broadcast(&customer_available);
    pthread_mutex_unlock(&queue_mutex);

    // Join clerk threads
    for (int i = 0; i < NUM_CLERKS; i++) {
        if (pthread_join(clerks_threads[i], NULL) != 0) {
             fprintf(stderr, "Failed to join clerk thread %d\n", i);
        }
    }

    // Calc final statistics!
    double avg_waiting_time_all;
    double avg_waiting_time_business;
    double avg_waiting_time_economy; 
    

    if (count_all > 0) {
        avg_waiting_time_all = total_waiting_time / count_all;
    } else {
        avg_waiting_time_all = 0.0;
    }
    
    if(count_business > 0) {
        avg_waiting_time_business = total_business_waiting_time / count_business;
    } else {  
        avg_waiting_time_business = 0.0;
    }
    
    if (count_economy > 0) {
        avg_waiting_time_economy = total_economy_waiting_time / count_economy;
    } else {
        avg_waiting_time_economy = 0.0;
    }

    // Output stats exactly as required!!
    printf("The average waiting time for all customers in the system is: %.2f seconds. \n", avg_waiting_time_all);
    printf("The average waiting time for all business-class customers is: %.2f seconds. \n", avg_waiting_time_business);
    printf("The average waiting time for all economy-class customers is: %.2f seconds. \n", avg_waiting_time_economy);

    // Cleanup
    pthread_mutex_destroy(&queue_mutex);
    pthread_mutex_destroy(&stats_mutex);
    pthread_cond_destroy(&customer_available);

    for (int i = 0; i < num_customers; i++) {
        pthread_cond_destroy(&customers[i].cond);
    }
    for (int i = 1; i <= NUM_CLERKS; i++) {
        pthread_cond_destroy(&clerk_cond[i]);
    }

    return 0;
}