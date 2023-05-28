#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

const int MAX_THREADS = 1024;

long thread_count;
double lower_bound, upper_bound;
int subintervals;
double total_sum;
sem_t sem;

/* Thread function */
void* Definite_Integral(void* rank);

/* Only executed by main thread */
void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);

void Read_data(double* lower_bound, double* upper_bound, int* subintervals);
double Trap(double left_endpoint, double right_endpoint, int trap_count, double base_len);

int main(int argc, char* argv[]) {
    long       thread;  /* Use long in case of a 64-bit system */
    pthread_t* thread_handles;

    /* Get number of threads from command line */
    Get_args(argc, argv);
    
    /* Input is as follows:
        lower bound of the integral
        upper bound of the integral
        number of subintervals (trapezoids) */
    Read_data(&lower_bound, &upper_bound, &subintervals);

    /* allocate array for threads */
    thread_handles = malloc(thread_count*sizeof(pthread_t));

    /* initialize semaphore */
    sem_init(&sem, 0, 1);

    for(thread = 0; thread < thread_count; thread++) {
        pthread_create(&thread_handles[thread], NULL, Definite_Integral, (void*) thread);
    }

    for(thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handles[thread], NULL);
    }

    printf("With n = %d trapezoids, our estimate\n", subintervals);
    printf("of the integral from %f to %f = %.15e\n", lower_bound, upper_bound, total_sum);

    sem_destroy(&sem);
    free(thread_handles);

    return 0;
} /* main */

void Read_data(double* lower_bound, double* upper_bound, int* subintervals) {
    printf("Enter a, b, and n:\n");
    scanf("%lf %lf %d", lower_bound, upper_bound, subintervals);
} /* Read_data */

double Trap(double left_endpoint, double right_endpoint, int trap_count, double base_len) {
    double estimate, x;
    int i;

    estimate = (left_endpoint + right_endpoint) / 2.0;
    
    for(i = 1; i <= trap_count-1; i++) {
        x = left_endpoint + i * base_len;
        estimate += x;
    }
    
    estimate = estimate * base_len;

    return estimate;
} /* Trap */

void* Definite_Integral(void* rank) {
    long my_rank = (long) rank;
    int local_subintervals = (subintervals / thread_count);
    double h = (upper_bound - lower_bound)/subintervals;
    
    double local_lower_bound = lower_bound + my_rank * local_subintervals * h;
    double local_upper_bound = local_lower_bound + local_subintervals * h;    
    double local_sum = Trap(local_lower_bound, local_upper_bound, local_subintervals, h);

    sem_wait(&sem);
    total_sum += local_sum;
    sem_post(&sem);
   
    return NULL;
} /* Definite_Integral */

void Get_args(int argc, char* argv[]) {
    if(argc != 2)
         Usage(argv[0]);
    
    thread_count = strtol(argv[1], NULL, 10);  
    
    if(thread_count <= 0 || thread_count > MAX_THREADS)
        Usage(argv[0]);
} /* Get_args */

void Usage(char* prog_name) {
   fprintf(stderr, "usage: %s <number of threads>\n", prog_name);
   exit(0);
} /* Usage */
