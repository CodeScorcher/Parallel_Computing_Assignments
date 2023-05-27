#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void read_row_col(int my_rank, int comm_size, int* rows, int* local_rows, int* columns);
void read_mat_vect(int my_rank, int comm_size, int rows, int local_rows, int columns, double local_mat[], double local_vect[]);
void mat_vect_mult(int order_n, int local_rows, double local_mat[], double local_vect[], double matrix_vect_result[]);
void print_matrix(int my_rank, int rows, int local_rows, int columns, char title[], double local_matrix[]);
void print_vector(int my_rank, int local_rows, int columns, char title[], double local_vect[]);

int main(void) {
    int my_rank, comm_size, rows, columns, local_rows;
    double *local_matrix, *local_vect, *matrix_vect_result;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    /* Have process 0 read the number of rows and columns of the square matrix, with one of the dimensions defining the vector size */
    read_row_col(my_rank, comm_size, &rows, &local_rows, &columns);

    local_matrix = malloc(local_rows * columns * sizeof(double));
    local_vect = malloc(local_rows * sizeof(double));

    /* Have process 0 read the input for the matrix and the vector, while also distributing this input data among the processes */
    read_mat_vect(my_rank, comm_size, rows, local_rows, columns, local_matrix, local_vect);
    
    /* Have process 0 print out the vector and matrix retrieved from user input */
    print_matrix(my_rank, rows, local_rows, columns, "Matrix: ", local_matrix);
    print_vector(my_rank, columns, local_rows, "Vector: ", local_vect);

    matrix_vect_result = malloc(columns * sizeof(double));

    /* Have other processes perform matrix-vector multiplication */
    mat_vect_mult(columns, local_rows, local_matrix, local_vect, matrix_vect_result);
    
    /* Have process 0 print out matrix-vector product */
    print_vector(my_rank, columns, local_rows, "Matrix-Vector Product: ", matrix_vect_result);

    free(local_matrix);
    free(local_vect);
    free(matrix_vect_result);

    MPI_Finalize();
    return 0;
}

void read_row_col(int my_rank, int comm_size, int* rows, int* local_rows, int* columns) {
    if (my_rank == 0) {
        printf("Enter the number of rows:\n");
        scanf("%d", rows);
         
        printf("Enter the number of columns:\n");
        scanf("%d", columns);
        
        *local_rows = (*rows / comm_size);

        MPI_Bcast(local_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(columns, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    else {
        MPI_Bcast(local_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(columns, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
}  /* read_row_col */

void read_mat_vect(int my_rank, int comm_size, int rows, int local_rows, int columns, double local_mat[], double local_vect[]) {
    double* matrix = NULL;
    double* vector = NULL;
    int i;

    if(my_rank == 0) {
        matrix = malloc(rows * columns * sizeof(double));
        vector = malloc(columns * sizeof(double));

        printf("Enter the square matrix:\n");
        for(i = 0; i < rows * columns; i++) {
            scanf("%lf", &matrix[i]);
        }
        
        printf("Enter the vector:\n");
        for(i = 0; i < columns; i++) {
            scanf("%lf", &vector[i]);
        }
        
        MPI_Scatter(matrix, local_rows*columns, MPI_DOUBLE, local_mat, local_rows*columns, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Scatter(vector, local_rows, MPI_DOUBLE, local_vect, local_rows, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        free(matrix);
        free(vector);
    }
    else {
        MPI_Scatter(matrix, local_rows*columns, MPI_DOUBLE, local_mat, local_rows*columns, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Scatter(vector, local_rows, MPI_DOUBLE, local_vect, local_rows, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
} /* read_mat_vect */

void mat_vect_mult(int columns, int local_rows, double local_mat[], double local_vect[], double matrix_vect_result[]) {
    double* vector = malloc(columns * sizeof(double));
    int i, j;

    MPI_Allgather(local_vect, local_rows, MPI_DOUBLE, vector, local_rows, MPI_DOUBLE, MPI_COMM_WORLD);

    for(i = 0; i < local_rows; i++) {
        matrix_vect_result[i] = 0.0;
        
        for(j = 0; j < columns; j++) {
            matrix_vect_result[i] += local_mat[i * columns + j] * vector[j];
        }
    }

    free(vector);
} /* mat_vect_mult */

void print_matrix(int my_rank, int rows, int local_rows, int columns, char title[], double local_matrix[]) {
    double* matrix = NULL;
    int i, j;

    if (my_rank == 0) {
        matrix = malloc(rows * columns * sizeof(double));
        
        MPI_Gather(local_matrix, local_rows*columns, MPI_DOUBLE, matrix, local_rows*columns, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        printf("%s\n", title);
        for(i = 0; i < rows; i++) {
            for(j = 0; j < columns; j++) {
                printf("%.2f ", matrix[i * columns + j]);
            }
            printf("\n");
        }

        free(matrix);
    }
    else {
        MPI_Gather(local_matrix, local_rows*columns, MPI_DOUBLE, matrix, local_rows*columns, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
} /* print_matrix */

void print_vector(int my_rank, int columns, int local_rows, char title[], double local_vect[]) {
    double* vector = NULL;
    int i;

    if (my_rank == 0) {
        vector = malloc(columns * sizeof(double));
        
        MPI_Gather(local_vect, local_rows, MPI_DOUBLE, vector, local_rows, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        printf("%s\n", title);
        for(i = 0; i < columns; i++)
            printf("%.2f ", vector[i]);
        printf("\n");

        free(vector);
    }
    else {
        MPI_Gather(local_vect, local_rows, MPI_DOUBLE, vector, local_rows, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
} /* print_vector */
