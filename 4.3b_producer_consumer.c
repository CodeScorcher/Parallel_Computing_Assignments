/* File:     
 *     4.3b_producer_consumer.c
 *
 * Purpose:  
 *     Implement producer-consumer synchronization with two threads using a mutex
 *
 * Input:
 *     number of threads
 *
 * Output:
 *     message
 *
 * Compile:  gcc -g -Wall -o 4.3b_producer_consumer 4.3b_producer_consumer.c -lpthread
 * 
 * Usage:
 *     4.3b_producer_consumer <number of threads>
 *
 * Notes:  
 *	- Each thread will create and print out message
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

const int MAX_THREADS = 1024;
const int MAX_STRING = 99;

int thread_count;
int msg = 0;
// int send = 1;
int recv = 0;
char* message;
pthread_mutex_t mutex;

void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);
void* Thread_work(void* rank); /* Thread function */

/*-----------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	long thread;
	pthread_t* thread_handles;
	message = malloc(MAX_STRING*sizeof(char));
	
	/* Get number of threads from command line */
   	Get_args(argc, argv);
	
	/* allocate array for threads */
	thread_handles = malloc(thread_count*sizeof(pthread_t));
	
	/* initialize mutex */
	pthread_mutex_init(&mutex, NULL);
	
	/* start threads */
	for(thread = 0; thread < thread_count; thread++) {
		pthread_create(&thread_handles[thread], NULL, Thread_work, (void*) thread);
	}
	
	/* wait for threads to complete */
	for(thread = 0; thread < thread_count; thread++) {
		pthread_join(thread_handles[thread], NULL);
	}
	
	pthread_mutex_destroy(&mutex);
    free(message);
	free(thread_handles);
	
	return 0;
} /* main */

/*-------------------------------------------------------------------
 * Function:    Thread_work
 * Purpose:     Even-ranked threads --> Producer: create msg
 *				Odd-ranked threads  --> Consumer: print out msg
 * In arg:      rank
 * Global var:  mutex, msg, message
 */
void *Thread_work(void* rank) {
	long my_rank = (long) rank;
	long dest = (my_rank + 1) % thread_count;
	long source = (my_rank + thread_count - 1) % thread_count;
	
	while(1) {
		pthread_mutex_lock(&mutex);
		if (recv) {
			if (msg) {
				printf("Th %ld > message: %s from %ld\n", my_rank, message, source);
				pthread_mutex_unlock(&mutex);
				break;
			}
		}
		else {
			sprintf(message, "hello");
			msg = 1;
            recv = 1;
			pthread_mutex_unlock(&mutex);
		}
        
		pthread_mutex_unlock(&mutex);
	}
	
	return NULL;
} /* Thread_work */

/*------------------------------------------------------------------
 * Function:    Get_args
 * Purpose:     Get the command line args
 * In args:     argc, argv
 * Globals out: thread_count, n
 */
void Get_args(int argc, char* argv[]) {
   if (argc != 2) Usage(argv[0]);
   thread_count = strtol(argv[1], NULL, 10);
   if (thread_count <= 0 || thread_count > MAX_THREADS) Usage(argv[0]);
} /* Get_args */


/*--------------------------------------------------------------------
 * Function:    Usage
 * Purpose:     Print command line for function and terminate
 * In arg:      prog_name
 */
void Usage(char* prog_name) {
	fprintf(stderr, "usage: %s <number of threads>\n", prog_name);
   	fprintf(stderr, "   number of threads represents the number of producers/consumers and should be > 1\n");
	exit(0);
} /* Usage */