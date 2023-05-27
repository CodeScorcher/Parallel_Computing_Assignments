#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void Read_n(int *n_p, int *local_n_p, int my_rank, int comm_sz, MPI_Comm comm);
void Check_for_error(int local_ok, char fname[], char message[], MPI_Comm comm);
void Read_data(double local_vec1[], double local_vec2[], double *scalar_p, int local_n, int my_rank, int comm_sz, MPI_Comm comm);
void Print_vector(double local_vec[], int local_n, int n, char title[], int my_rank, MPI_Comm comm);
double Par_dot_product(double local_vec1[], double local_vec2[], int local_n, MPI_Comm comm);
void Par_vector_scalar_mult(double local_vec[], double scalar, double local_result[], int local_n);

int main(void) {
    int n, local_n;
    double *local_vec1, *local_vec2;
    double scalar;
    double *local_scalar_mult1, *local_scalar_mult2;
    double dot_product = 0.0, par_dot_product = 0.0;
    int comm_sz, my_rank;
    // int local_ok;
    double *a = NULL;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm comm = MPI_COMM_WORLD;
    
    Read_n(&n, &local_n, my_rank, comm_sz, comm);
    // local_ok = 0;
    // Check_for_error(local_n, "Read_n()", "wrong input", comm);
    local_vec1 = malloc(local_n * sizeof(double));
    local_vec2 = malloc(local_n * sizeof(double));
    
    Read_data(local_vec1, local_vec2, &scalar, local_n, my_rank, comm_sz, comm);
    
    /* Print input data */
    Print_vector(local_vec1, local_n, n, "vec1", my_rank, comm);
    Print_vector(local_vec2, local_n, n, "vec2", my_rank, comm);
    
    /* Compute and print dot product */
    par_dot_product = Par_dot_product(local_vec1, local_vec2, local_n, comm);
    
    // now we have comm_sz times par_dot_product
    if (my_rank == 0) {
        // int i;
        // a = malloc(comm_sz * sizeof(double));
        
        // Collect all of the components of the vector onto process 0,
        // and then process 0 can process all of the components.
        // MPI_Gather(&par_dot_product, 1, MPI_DOUBLE, a, 1, MPI_DOUBLE, 0, comm);
        MPI_Reduce(&par_dot_product, &dot_product, 1, MPI_DOUBLE, MPI_SUM, 0, comm);
        
        /* 
        for(i = 0; i < comm_sz; i++) {
            printf("a[%d] %lf\n", i, a[i]);
            dot_product = dot_product + a[i];
        }
        */
        
        printf("dot product: %lf", dot_product);
        printf("\n");
        free(a);
    }
    else {
        // MPI_Gather(&par_dot_product, 1, MPI_DOUBLE, a, 1, MPI_DOUBLE, 0, comm);
        MPI_Reduce(&par_dot_product, &dot_product, 1, MPI_DOUBLE, MPI_SUM, 0, comm);
    }
    
    local_scalar_mult1 = malloc(local_n * sizeof(double));
    local_scalar_mult2 = malloc(local_n * sizeof(double));
    
    /* Compute scalar multiplication and print out result */
    Par_vector_scalar_mult(local_vec1, scalar, local_scalar_mult1, local_n);
    Par_vector_scalar_mult(local_vec2, scalar, local_scalar_mult2, local_n);
    Print_vector(local_scalar_mult1, local_n, n, "vec1 after scalar multi", my_rank, comm);
    Print_vector(local_scalar_mult2, local_n, n, "vec2 after scalar multi", my_rank, comm);
    
    free(local_scalar_mult2);
    free(local_scalar_mult1);
    free(local_vec2);
    free(local_vec1);
    
    MPI_Finalize();
    return 0;
}


/*-------------------------------------------------------------------*/
void Check_for_error(
    int local_ok /* in */,
    char fname[] /* in */,
    char message[] /* in */,
    MPI_Comm comm /* in */) {
    int ok;
    MPI_Allreduce(&local_ok, &ok, 1, MPI_INT, MPI_MIN, comm);
    if (ok == 0) {
        int my_rank;
        MPI_Comm_rank(comm, &my_rank);
        if (my_rank == 0) {
            fprintf(stderr, "Proc %d > In %s, %s\n", my_rank, fname, message);
            fflush(stderr);
        }
        MPI_Finalize();
        exit(-1);
    }
} /* Check_for_error */


/* Get the input of n: size of the vectors, and then calculate local_n according to comm_sz and n */
/* where local_n is the number of elements each process obtains */
/*-------------------------------------------------------------------*/
void Read_n(int *n_p, int *local_n_p, int my_rank, int comm_sz, MPI_Comm comm) {
    if (my_rank == 0) {
        printf("Enter the size of the vectors\n");
        scanf("%d", n_p);
        *local_n_p = *n_p / comm_sz;
    }
    // Data belonging to a process 0 is sent to all of the processes in the communicator.
    MPI_Bcast(n_p, 1, MPI_DOUBLE, 0, comm);
    MPI_Bcast(local_n_p, 1, MPI_DOUBLE, 0, comm);
} /* Read_n */


/* local_vec1 and local_vec2 are the two local vectors of size local_n which the process pertains */
/* process 0 will take the input of the scalar, the two vectors a and b */
/* process 0 will scatter the two vectors a and b across all processes */
/*-------------------------------------------------------------------*/
void Read_data(double local_vec1[], double local_vec2[], double *scalar_p, int local_n, int my_rank, int comm_sz, MPI_Comm comm) {
    double *a, *b;
    int n, i;
    if (my_rank == 0) {
        printf("What is the scalar?\n");
        scanf("%lf", scalar_p);
    }

    // Data belonging to a process 0 is sent to all of the processes in the communicator.
    MPI_Bcast(scalar_p, 1, MPI_DOUBLE, 0, comm);
    
    if (my_rank == 0) {
        n = local_n * comm_sz;
        a = malloc(n * sizeof(double));
        b = malloc(n * sizeof(double));

        printf("The calculation will be done parallel by %d processes.\n", comm_sz);
        printf("Each process works with %d elements out of %d.\n", local_n, n);
        
        printf("Enter the first vector\n");
        for (i = 0; i < n; i++)
            scanf("%lf", &a[i]);
        
        printf("Enter the second vector\n");
        for (i = 0; i < n; i++)
            scanf("%lf", &b[i]);
        
        /* MPI_Scatter reads in an entire vector on process 0 but only sends
        the needed components to each of the other processes. */
        MPI_Scatter(a, local_n, MPI_DOUBLE, local_vec1, local_n, MPI_DOUBLE, 0, comm);
        MPI_Scatter(b, local_n, MPI_DOUBLE, local_vec2, local_n, MPI_DOUBLE, 0, comm);
        
        free(a);
        free(b);
    }
    else {
        MPI_Scatter(a, local_n, MPI_DOUBLE, local_vec1, local_n, MPI_DOUBLE, 0, comm);
        MPI_Scatter(b, local_n, MPI_DOUBLE, local_vec2, local_n, MPI_DOUBLE, 0, comm);
    }
} /* Read_data */


/* The print_vector gathers the local vectors from all processes and print the
gathered vector */
/*-------------------------------------------------------------------*/
void Print_vector(double local_vec[], int local_n, int n, char title[], int my_rank, MPI_Comm comm) {
    double *a = NULL;
    int i;

    if (my_rank == 0) {
        a = malloc(n * sizeof(double));
        // Collect all of the components of the vector onto process 0,
        // and then process 0 can process all of the components.
        MPI_Gather(local_vec, local_n, MPI_DOUBLE, a, local_n, MPI_DOUBLE, 0, comm);
        
        printf("%s\n", title);
        for(i = 0; i < n; i++)
            printf("%f ", a[i]);
        printf("\n");
        
        free(a);
    }
    else {
        MPI_Gather(local_vec, local_n, MPI_DOUBLE, a, local_n, MPI_DOUBLE, 0, comm);
    }
} /* Print_vector */


/* This function computes and returns the partial dot product of local_vec1 and
local_vec2 */
/*-------------------------------------------------------------------*/
double Par_dot_product(double local_vec1[], double local_vec2[], int local_n, MPI_Comm comm) {
    double res = 0.0;
    int i;

    for (i = 0; i < local_n; i++) {
        res = res + (local_vec1[i] * local_vec2[i]);
    }
    
    return res;
} /* Par_dot_product */


/* This function gets the vector which is the scalar times local_vec, and put the
vector into local_result */
/*-------------------------------------------------------------------*/
void Par_vector_scalar_mult(double local_vec[], double scalar, double local_result[], int local_n) {
    int i;
    for (i = 0; i < local_n; i++) {
        local_result[i] = scalar * local_vec[i];
    }
} /* Par_vector_scalar_mult */