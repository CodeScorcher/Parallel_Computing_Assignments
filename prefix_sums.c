#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void read_order(int my_rank, int comm_size, int* n, int* local_n);
void generate_data(int my_rank, int comm_size, int n, int local_n, int local_array[]);
void compute_prefix_sum(int my_rank, int local_n, int prefix_sum, int local_array[]);
void print_vector(int my_rank, int n, int local_n, char title[], int local_array[]);

int main(int argc, char **argv) {
    int my_rank, comm_size, n, local_n;
    int* numArray;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    
    /* Have process 0 read the size of the vector in order to determine how much data each process receives */
    read_order(my_rank, comm_size, &n, &local_n);
    
    int prefix_sum = 0;
    numArray = malloc(n * sizeof(int));

    /* Generate an array of integers that we will use to compute our sequence of partial prefix sums */
    generate_data(my_rank, comm_size, n, local_n, numArray);

    /* Compute partial prefix sums and assign results to the appropiate array locations */
    compute_prefix_sum(my_rank, local_n, prefix_sum, numArray);

    /* Have process 0 print out the array after it has been modified by the compute_prefix_sum() function */
    print_vector(my_rank, n, local_n, "Full Vector:", numArray);

    free(numArray);

    MPI_Finalize();
    return 0;
}

void read_order(int my_rank, int comm_size, int* n, int* local_n) {
   if (my_rank == 0) {
      printf("How many numbers do you want?\n");
      scanf("%d", n);

      *local_n = (*n / comm_size);

      MPI_Bcast(local_n, 1, MPI_INT, 0, MPI_COMM_WORLD);
   }
   else {
      MPI_Bcast(local_n, 1, MPI_INT, 0, MPI_COMM_WORLD);
   }
}  /* read_n */

void generate_data(int my_rank, int comm_size, int n, int local_n, int local_array[]) {
    int* data = NULL;
    int i;

    if(my_rank == 0) {
        data = malloc(n * sizeof(int));

        for(i = 1; i < n; i++) {
            data[i] = i;
        }

        MPI_Scatter(data, local_n, MPI_INT, local_array, local_n, MPI_INT, 0, MPI_COMM_WORLD);
        free(data);
    }
    else {
        MPI_Scatter(data, local_n, MPI_INT, local_array, local_n, MPI_INT, 0, MPI_COMM_WORLD);
    }
} /* generate_data */

void compute_prefix_sum(int my_rank, int local_n, int prefix_sum, int local_array[]) {
    int i;

    for(i = 0; i < local_n; i++) {
        if(i > 0) {
            local_array[i] += local_array[i-1];
        }
    }

    MPI_Scan(&local_array[local_n-1], &prefix_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    
    if(my_rank != 0) {
        for(i = 0; i < local_n; i++) {
            local_array[i] += prefix_sum - local_array[local_n-1];
        }
    }
} /* computer_prefix_sum */

void print_vector(int my_rank, int n, int local_n, char title[], int local_array[]) {
    int* a = NULL;
    int i;
    
    if (my_rank == 0) {
        a = malloc(n * sizeof(double));
        
        MPI_Gather(local_array, local_n, MPI_INT, a, local_n, MPI_INT, 0, MPI_COMM_WORLD);
        
        printf("%s\n[", title);
        for(i = 0; i < n; i++)
            printf("%d ", a[i]);
        printf("]\n");

        free(a);
    }
    else {
        MPI_Gather(local_array, local_n, MPI_INT, a, local_n, MPI_INT, 0, MPI_COMM_WORLD);
    }
} /* print_vector */