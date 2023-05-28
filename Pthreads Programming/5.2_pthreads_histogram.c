#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

const int MAX_THREADS = 1024;

long thread_count;
int n, data_count;
double lower_bound, upper_bound, bin_width;
double* data;
int* bin_counts;
sem_t sem;

/* Thread function */
void* update_bin_counts(void* rank);

/* Only executed by main thread */
void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);

void read_data(int* n, int* data_count, double* lower_bound, double* upper_bound);
void generate_data(int n, int data_count, double lower_bound, double upper_bound, double data[], int bin_counts[]);
int find_bin(double target, int n, double bin_width, double lower_bound);

int main(int argc, char* argv[]) {
    long       thread;  /* Use long in case of a 64-bit system */
    pthread_t* thread_handles;

    /* Have process 0 read in input data and distribute it among the processes */
    read_data(&n, &data_count, &lower_bound, &upper_bound);

    bin_width = (upper_bound - lower_bound) / n;
    data = malloc(data_count * sizeof(double));
    bin_counts = malloc(n * sizeof(int));

    /* Have process 0 generate random measurements and distribute them among the processes */
    generate_data(n, data_count, lower_bound, upper_bound, data, bin_counts);
    
    /* Get number of threads from command line */
    Get_args(argc, argv);

    /* allocate array for threads */
    thread_handles = malloc(thread_count*sizeof(pthread_t));

    /* initialize semaphore */
    sem_init(&sem, 0, 1);

    for(thread = 0; thread < thread_count; thread++) {
        pthread_create(&thread_handles[thread], NULL, update_bin_counts, (void*) thread);
    }

    for(thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handles[thread], NULL);
    }

    printf("\n");
    for(int j = 0; j < n; j++) {
        printf("[  %0.3lf|  %0.3lf) %d\n", lower_bound + (bin_width * j), lower_bound + (bin_width * (j + 1)), bin_counts[j]);
    }

    sem_destroy(&sem);
    free(thread_handles);

    return 0;
} /* main */

void read_data(int* n, int* data_count, double* lower_bound, double* upper_bound) {
    printf("Enter the number of bins:\n");
    scanf("%d", n);

    printf("Enter minimum value:\n");
    scanf("%lf", lower_bound);

    printf("Enter maximum value:\n");
    scanf("%lf", upper_bound);

    printf("Number of random values to be used (Value should be divisible by 4):\n");
    scanf("%d", data_count);
}  /* read_data */

void generate_data(int n, int data_count, double lower_bound, double upper_bound, double data[], int bin_counts[]) {
    int i;

    srand(time(NULL));

    for(i = 0; i < data_count; i++) {
        /* Measurements are randomly generated based on input for data_count, a, and b */
        data[i] = ((double)rand() * (upper_bound - lower_bound))/(double)RAND_MAX + lower_bound;
        printf("\n%lf", data[i]);
    }

    for(i = 0; i < n; i++) {
        bin_counts[i] = 0;
    }
} /* generate_data */

int find_bin(double target, int n, double bin_width, double lower_bound) {
    int i;

    for(i = 0; i < n; i++) {
        if(target >= (lower_bound + (bin_width * i)) && target < (lower_bound + (bin_width * (i + 1)))) {
            return i;
        }
    }

    return -1;
} /* find_bin */

void* update_bin_counts(void* rank) {
    long my_rank = (long) rank;
    int local_data_count = (data_count / thread_count);
    int my_first_i = local_data_count*my_rank;
    int my_last_i = my_first_i + local_data_count;
    int* local_bin_counts = NULL;
    int i, bin;

    local_bin_counts = malloc(n * sizeof(double));

    /* Initialize local bin array */
    for(i = 0; i < n; i++) {
        local_bin_counts[i] = 0;
    }

    for(i = my_first_i; i < my_last_i; i++) {
        /* Update local_bin_counts on the basis of its assigned elements */
        bin = find_bin(data[i], n, bin_width, lower_bound);
        local_bin_counts[bin]++;
    }

    for(int i = 0; i < n; i++) {
        sem_wait(&sem);
        bin_counts[i] += local_bin_counts[i];
        sem_post(&sem);
    }
   
    return NULL;
} /* update_bin_counts */

void Get_args(int argc, char* argv[]) {
    // if(argc != 7)
    //     Usage(argv[0]);
    
    thread_count = strtol(argv[1], NULL, 10);  
    
    if(thread_count <= 0 || thread_count > MAX_THREADS)
        Usage(argv[0]);
} /* Get_args */

void Usage(char* prog_name) {
   fprintf(stderr, "usage: %s <number of threads> <n>\n", prog_name);
   fprintf(stderr, "   n is the number of terms and should be >= 1\n");
   fprintf(stderr, "   n should be evenly divisible by the number of threads\n");
   exit(0);
} /* Usage */
