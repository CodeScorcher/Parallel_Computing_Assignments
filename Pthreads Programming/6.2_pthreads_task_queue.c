#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

const int MAX_THREADS = 1024;

long task_count;
long thread_count;

/* Our shared data structure is a sorted linked list of integers */
struct list_node_s* linked_list;

pthread_rwlock_t rwlock;

/* Thread function */
void* Execute_Tasks(void* rank);

/* Only executed by main thread */
void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);

/* Linked list operations of interest are Member, Insert, and Delete */
int Member(int value, struct list_node_s* head_p);
int Insert(int value, struct list_node_s** head_pp);
int Delete(int value, struct list_node_s** head_pp);

int main(int argc, char* argv[]) {
    long       thread;
    pthread_t* thread_handles;

    /* Get number of tasks and threads from command line */
    Get_args(argc, argv);

    /* Allocate array for threads */
    thread_handles = malloc(thread_count*sizeof(pthread_t));

    pthread_rwlock_init(&rwlock, NULL);

    for(thread = 0; thread < thread_count; thread++) {
        pthread_create(&thread_handles[thread], NULL, Execute_Tasks, (void*) thread);
    }

    for(thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handles[thread], NULL);
    }

    pthread_rwlock_destroy(&rwlock);
    free(thread_handles);

    return 0;
} /* main */

struct list_node_s {
    int data;
    struct list_node_s* next;
};

int Member(int value, struct list_node_s* head_p) {
    struct list_node_s* curr_p = head_p;

    while(curr_p != NULL && curr_p-> data < value) {
        curr_p = curr_p->next;
    }

    if(curr_p == NULL || curr_p->data > value) {
        printf("MEMBER: Value %d does NOT exist in the linked list!\n", value);
        return 0;
    }
    else {
        printf("MEMBER: Value %d exists in the linked list!\n", value);
        return 1;
    }
} /* Member */

int Insert(int value, struct list_node_s** head_pp) {
    struct list_node_s* curr_p = *head_pp;
    struct list_node_s* pred_p = NULL;
    struct list_node_s* temp_p;

    while(curr_p != NULL && curr_p->data < value) {
        pred_p = curr_p;
        curr_p = curr_p->next;
    }

    if(curr_p == NULL || curr_p->data > value) {
        /* Create temporary node that stores a value and a pointer to the next node */
        temp_p = malloc(sizeof(struct list_node_s));
        temp_p->data = value;
        temp_p->next = curr_p;

        /* If the previous node is empty, then we assign the temporary node to our head node */
        if(pred_p == NULL)
            *head_pp = temp_p;
        /* If the previous node isn't empty, then we assign the temporary node to our previous node */
        else
            pred_p->next = temp_p;

        printf("INSERT: Sucessfully inserted new node with value %d!\n", value);
        
        return 1;
    }
    /* Value already in list */
    else {
        printf("INSERT: Failed to insert new node with value %d!\n", value);
        return 0;
    }
} /* Insert */

int Delete(int value, struct list_node_s** head_pp) {
    struct list_node_s* curr_p = *head_pp;
    struct list_node_s* pred_p = NULL;

    while(curr_p != NULL && curr_p->data < value) {
        pred_p = curr_p;
        curr_p = curr_p->next;
    }

    if(curr_p != NULL && curr_p->data == value) {
        /* Deleting first node in list */
        if(pred_p == NULL) {
            *head_pp = curr_p->next;
            free(curr_p);
        }
        else {
            pred_p->next = curr_p->next;
            free(curr_p);
        }

        printf("DELETE: Sucessfully deleted node with value %d!\n", value);

        return 1;
    }
    /* Value isn't in the list */
    else {
        printf("DELETE: Failed to delete node with value %d!\n", value);
        return 0;
    }
} /* Delete */

void* Execute_Tasks(void* rank) {
    int local_tasks = (task_count / thread_count);
    int task_number, value;
    srand(time(NULL));

    for(int i = 0; i < local_tasks; i++) {
        task_number = (rand() % 3) + 1;
        value = (rand() % 20) + 1;

        if(task_number == 1) {
            pthread_rwlock_rdlock(&rwlock);
            Member(value, linked_list);
            pthread_rwlock_unlock(&rwlock);
        }
        else if(task_number == 2) {
            pthread_rwlock_wrlock(&rwlock);
            Insert(value, &linked_list);
            pthread_rwlock_unlock(&rwlock);
        }
        else if(task_number == 3) {
            pthread_rwlock_wrlock(&rwlock);
            Delete(value, &linked_list);
            pthread_rwlock_unlock(&rwlock);
        }
    }
    
    return NULL;
} /* Execute_Tasks */

void Get_args(int argc, char* argv[]) {
    if(argc != 3)
        Usage(argv[0]);

    task_count = strtol(argv[1], NULL, 10);
    
    if(task_count <= 0)
        Usage(argv[0]);

    thread_count = strtoll(argv[2], NULL, 10);

    if(thread_count <= 0 || thread_count > MAX_THREADS)
        Usage(argv[0]);
} /* Get_args */

void Usage(char* prog_name) {
    fprintf(stderr, "usage: %s <number of tasks> <number of threads>\n", prog_name);
    fprintf(stderr, "   number of threads and tasks should be >= 1\n");
    fprintf(stderr, "   number of tasks should also be evenly divisible by the number of threads\n");
    exit(0);
} /* Usage */
