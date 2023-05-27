#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>

const int MAX_THREADS = 1024;

long thread_count;
int rows, columns;
double* matrix; 
double* vector;
double* matrix_vect_result;
sem_t sem;

/* Only executed by main thread */
void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);
void *Pth_mat_vect(void* rank);

void read_row_col(int* rows, int* columns);
void read_mat_vect(int rows, int columns, double matrix[], double vector[]);
void print_matrix(int rows, int columns, char title[], double matrix[]);
void print_vector(int columns, char title[], double vector[]);

int main(int argc, char* argv[]) {
    long       thread;
    pthread_t* thread_handles;
    
    /* Have process 0 read the number of rows and columns of the square matrix, with one of the dimensions defining the vector size */
    read_row_col(&rows, &columns);
    
    matrix = malloc(rows*columns*sizeof(double));
    vector = malloc(columns*sizeof(double));
    matrix_vect_result = malloc(columns*sizeof(double));

    /* Have process 0 read the input for the matrix and the vector, while also distributing this input data among the processes */
    read_mat_vect(rows, columns, matrix, vector);

    /* Get number of threads from command line */
    Get_args(argc, argv);

    /* allocate array for threads */
	thread_handles = malloc(thread_count*sizeof(pthread_t));
    
    /* initialize semaphore */
    sem_init(&sem, 0, 1);
    
    for(thread = 0; thread < thread_count; thread++) {
        pthread_create(&thread_handles[thread], NULL, Pth_mat_vect, (void*) thread);
    }
    
    for(thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handles[thread], NULL);
    }
    
    /* Have process 0 print out the vector and matrix retrieved from user input */
    print_matrix(rows, columns, "Matrix: ", matrix);
    print_vector(columns, "Vector: ", vector);
    
    /* Have process 0 print out matrix-vector product */
    print_vector(columns, "Matrix-Vector Product: ", matrix_vect_result);

    sem_destroy(&sem);
    free(thread_handles);
    free(matrix_vect_result);

    return 0;
}

void read_row_col(int* rows, int* columns) {
        printf("Enter the number of rows:\n");
        scanf("%d", rows);
         
        printf("Enter the number of columns:\n");
        scanf("%d", columns);
}  /* read_row_col */

void read_mat_vect(int rows, int columns, double matrix[], double vector[]) {
    int i;

    printf("Enter the square matrix:\n");
    for(i = 0; i < rows * columns; i++) {
        scanf("%lf", &matrix[i]);
    }
    
    printf("Enter the vector:\n");
    for(i = 0; i < columns; i++) {
        scanf("%lf", &vector[i]);
    }
} /* read_mat_vect */

void *Pth_mat_vect(void* rank) {
    long my_rank = (long) rank;
    int local_rows = (rows / thread_count);
    
    int my_first_row = my_rank*local_rows;
    int my_last_row = (my_rank + 1) * local_rows - 1;
    int i, j;

    for(i = my_first_row; i <= my_last_row; i++) {
        matrix_vect_result[i] = 0.0;

        for(j = 0; j < columns; j++) {
            sem_wait(&sem);
            matrix_vect_result[i] += matrix[i * columns + j] * vector[j];
            sem_post(&sem);
        }
    }

    return NULL;
}

void print_matrix(int rows, int columns, char title[], double matrix[]) {
    int i, j;

    printf("%s\n", title);
    for(i = 0; i < rows; i++) {
        for(j = 0; j < columns; j++) {
            printf("%.2f ", matrix[i * columns + j]);
        }
        printf("\n");
    }
} /* print_matrix */

void print_vector(int columns, char title[], double vector[]) {
    int i;
    
    printf("%s\n", title);
    for(i = 0; i < columns; i++)
        printf("%.2f ", vector[i]);
    printf("\n");
} /* print_vector */

void Get_args(int argc, char* argv[]) {
    //    if(argc != 3)
    //     Usage(argv[0]);

    thread_count = strtol(argv[1], NULL, 10);

    if(thread_count <= 0 || thread_count > MAX_THREADS)
        Usage(argv[0]);
} /* Get_args */

void Usage(char* prog_name) {
	fprintf(stderr, "usage: %s <number of threads>\n", prog_name);
   	fprintf(stderr, "   number of threads represents the number of producers/consumers and should be > 1\n");
	exit(0);
} /* Usage */