#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

const int MAX_THREADS = 1024;

long thread_count;
long long int tosses, local_tosses;
long long int total_number_in_circle = 0;
double pi_estimate;
sem_t sem;

void* monte_carlo(void* rank);
void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);

int main(int argc, char* argv[]) {
    long thread;
	pthread_t* thread_handles;

    /* Get number of threads from command line */
    Get_args(argc, argv);

    /* allocate array for threads */
	thread_handles = malloc(thread_count*sizeof(pthread_t));

    /* initialize semaphore */
    sem_init(&sem, 0, 1);

    /* start threads */
	for(thread = 0; thread < thread_count; thread++) {
		pthread_create(&thread_handles[thread], NULL, monte_carlo, (void*) thread);
	}
	
	/* wait for threads to complete */
	for(thread = 0; thread < thread_count; thread++) {
		pthread_join(thread_handles[thread], NULL);
	}

    pi_estimate = (4.0*(double)total_number_in_circle)/((double) tosses);
    printf("Result: %lf", pi_estimate);

    sem_destroy(&sem);
    free(thread_handles);

    return 0;
}

void* monte_carlo(void* rank) {
    // long my_rank = (long) rank;
    long long int toss;
    double x, y, distance_squared;
    local_tosses = (tosses / thread_count);
    long long int local_number_in_circle = 0;
    
    srand(time(NULL));

    for(toss = 0; toss < local_tosses; toss++) {
        x = (((double)rand() * (2 - (0)))/(double)RAND_MAX + (0)) - 1;
        y = (((double)rand() * (2 - (0)))/(double)RAND_MAX + (0)) - 1;

        distance_squared = x*x + y*y;

        if(distance_squared <= 1) {
            (local_number_in_circle)++;
        }
    }

    sem_wait(&sem);
    total_number_in_circle += local_number_in_circle;
    sem_post(&sem);

    return NULL;
}

void Get_args(int argc, char* argv[]) {
   if (argc != 3) 
      Usage(argv[0]);
   
   thread_count = strtol(argv[1], NULL, 10);
   
   if (thread_count <= 0 || thread_count > MAX_THREADS)
      Usage(argv[0]);
   
   tosses = strtoll(argv[2], NULL, 10);
   
   if (tosses <= 0)
      Usage(argv[0]);
} /* Get_args */

void Usage(char* prog_name) {
   fprintf(stderr, "usage: %s <number of threads> <number of tosses>\n", prog_name);
   fprintf(stderr, "   number of tosses should be >= 1\n");
   fprintf(stderr, "   number of tosses should be evenly divisible by the number of threads\n");
   exit(0);
} /* Usage */