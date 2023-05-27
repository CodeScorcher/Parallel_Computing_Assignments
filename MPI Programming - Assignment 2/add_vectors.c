#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void read_order(int my_rank, int comm_size, int* n, int* local_n);
void read_matrix_vect(int my_rank, int comm_size, int local_n, double local_vec1[], double local_vec2[], MPI_Datatype type);
void mat_vect_mult(int local_n, double local_vec1[], double local_vec2[], double vector_result[]);
void print_vector(int my_rank, int n, int local_n, char title[], double local_vec[], MPI_Datatype type);

int main(void) {
    int my_rank, comm_size, n, local_n;
    double *local_vec1, *local_vec2, *vector_result;
    MPI_Datatype type;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    /* MPI_TYPE_CONTIGUOUS is a datatype constructor that allows replication of a datatype into contiguous locations */
    MPI_Type_contiguous(1, MPI_DOUBLE, &type);
    
    /* MPI datatype is committed through MPI_Type_commit in order for it to be using during communication */
    MPI_Type_commit(&type);

    /* Have process 0 read the size of the vector in order to determine how much data each process receives */
    read_order(my_rank, comm_size, &n, &local_n);

    local_vec1 = malloc(local_n * sizeof(double));
    local_vec2 = malloc(local_n * sizeof(double));

    /* Have process 0 read the input vector and distribute vector data among the processes */
    read_matrix_vect(my_rank, comm_size, local_n, local_vec1, local_vec2, type);

    /* Have process 0 print out the input vectors */
    print_vector(my_rank, n, local_n, "Vector 1: ", local_vec1, type);
    print_vector(my_rank, n, local_n, "Vector 2: ", local_vec2, type);

    vector_result = malloc(n * sizeof(double));

    /* Computer vector sum and have process 0 print out the result */
    mat_vect_mult(local_n, local_vec1, local_vec2, vector_result);
    print_vector(my_rank, n, local_n, "Vector Sum: ", vector_result, type);

    free(local_vec1);
    free(local_vec2);
    free(vector_result);

    MPI_Finalize();
    return 0;
}

void read_order(int my_rank, int comm_size, int* n, int* local_n) {
   if (my_rank == 0) {
      printf("What is the size of the vector?\n");
      scanf("%d", n);

      *local_n = (*n / comm_size);

      MPI_Bcast(local_n, 1, MPI_INT, 0, MPI_COMM_WORLD);
   }
   else {
      MPI_Bcast(local_n, 1, MPI_INT, 0, MPI_COMM_WORLD);
   }
}  /* read_n */

void read_matrix_vect(int my_rank, int comm_size, int local_n, double local_vec1[], double local_vec2[], MPI_Datatype type) {
    double* a = NULL;
    double* b = NULL;
    int i;

    if(my_rank == 0) {
        a = malloc(local_n * comm_size * sizeof(double));
        b = malloc(local_n * comm_size * sizeof(double));

        printf("Enter the first vector:\n");
        for(i = 0; i < local_n * comm_size; i++)
            scanf("%lf", &a[i]);
        
        printf("Enter the second vector:\n");
        for(i = 0; i < local_n * comm_size; i++)
            scanf("%lf", &b[i]);
        
        MPI_Scatter(a, local_n, type, local_vec1, local_n, type, 0, MPI_COMM_WORLD);
        MPI_Scatter(b, local_n, type, local_vec2, local_n, type, 0, MPI_COMM_WORLD);
        
        free(a);
        free(b);
    }
    else {
        MPI_Scatter(a, local_n, type, local_vec1, local_n, type, 0, MPI_COMM_WORLD);
        MPI_Scatter(b, local_n, type, local_vec2, local_n, type, 0, MPI_COMM_WORLD);
    }
} /* read_vector */

void mat_vect_mult(int local_n, double local_vec1[], double local_vec2[], double vector_result[]) {
    int i;

    for(i = 0; i < local_n; i++) {
        vector_result[i] = local_vec1[i] + local_vec2[i];
    }
} /* add_vectors */

void print_vector(int my_rank, int n, int local_n, char title[], double local_vec[], MPI_Datatype type) {
    double* a = NULL;
    int i;
    
    if (my_rank == 0) {
        a = malloc(n * sizeof(double));
        
        MPI_Gather(local_vec, local_n, type, a, local_n, type, 0, MPI_COMM_WORLD);
        
        printf("%s\n", title);
        for(i = 0; i < n; i++)
            printf("%.2f ", a[i]);
        printf("\n");

        free(a);
    }
    else {
        MPI_Gather(local_vec, local_n, type, a, local_n, type, 0, MPI_COMM_WORLD);
    }
} /* print_vector */
