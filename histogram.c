#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

void read_data(int my_rank, int comm_size, int* n, int* data_count, int* local_data_count, double* lower_bound, double* upper_bound);
void generate_data(int my_rank, int comm_size, int n, int data_count, int local_data_count, double lower_bound, double upper_bound, double local_data[], int bin_counts[]);
void print_data(int my_rank, int local_data_count, int data_count, int n, double local_data[], int bin_counts[]);
int find_bin(double target, int n, double bin_width, double lower_bound);
void update_bin_counts(int local_data_count, int n, double lower_bound, double local_data[], int bin_counts[], double bin_width);

int main(void) {
    /* Input is as follows:
        data_count - number of measurements
        lower_bound - lower bound of measurements
        upper_bound - upper bound of measurements
        n - number of bins 

       Output is as follows:
        The measurements
        The range of each bin
        The number of measurements in each bin
    */
    int my_rank, comm_size, n, data_count, local_data_count;
    double lower_bound, upper_bound, bin_width;
    double* local_data;
    int* bin_counts;                                                         
    
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    /* Have process 0 read in input data and distribute it among the processes */
    read_data(my_rank, comm_size, &n, &data_count, &local_data_count, &lower_bound, &upper_bound);

    bin_width = (upper_bound - lower_bound) / n;
    local_data = malloc(local_data_count * sizeof(double));
    bin_counts = malloc(n * sizeof(int));

    /* Have process 0 generate random measurements and distribute them among the processes */
    generate_data(my_rank, comm_size, n, data_count, local_data_count, lower_bound, upper_bound, local_data, bin_counts);

    /* Process 0 will also printout the results */
    print_data(my_rank, local_data_count, data_count, n, local_data, bin_counts);
    
    /* Find the bin to which an element of data belongs to and increment the appropiate entry in bin_counts */
    /* bin_counts consist of elements representing the number of occurences for a specific measurement */
    update_bin_counts(local_data_count, n, lower_bound, local_data, bin_counts, bin_width);

    if(my_rank == 0) {
        printf("\n");
        for(int i = 0; i < n; i++) {
            printf("[  %0.3lf|  %0.3lf) %d\n", lower_bound + (bin_width * i), lower_bound + (bin_width * (i + 1)), bin_counts[i]);
        }
    }

    MPI_Finalize();
    return 0;
}

void read_data(int my_rank, int comm_size, int* n, int* data_count, int* local_data_count, double* lower_bound, double* upper_bound) {
    if(my_rank == 0) {
        printf("Enter the number of bins:\n");
        scanf("%d", n);

        printf("Enter minimum value:\n");
        scanf("%lf", lower_bound);

        printf("Enter maximum value:\n");
        scanf("%lf", upper_bound);

        printf("Number of random values to be used (Value should be divisible by 4):\n");
        scanf("%d", data_count);
        
        *local_data_count = (*data_count / comm_size);

        MPI_Bcast(data_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(local_data_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(lower_bound, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(upper_bound, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    else {
        MPI_Bcast(data_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(local_data_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(lower_bound, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(upper_bound, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
}  /* read_data */

void generate_data(int my_rank, int comm_size, int n, int data_count, int local_data_count, double lower_bound, double upper_bound, double local_data[], int bin_counts[]) {
    double* data = NULL;
    int i;

    if(my_rank == 0) {
        srand(time(NULL));
        data = malloc(data_count * sizeof(double));

        for(i = 0; i < data_count; i++) {
            /* Measurements are randomly generated based on input for data_count, a, and b */
            data[i] = ((double)rand() * (upper_bound - lower_bound))/(double)RAND_MAX + lower_bound;
        }

        for(i = 0; i < n; i++) {
            bin_counts[i] = 0;
        }

        MPI_Bcast(bin_counts, n, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatter(data, local_data_count, MPI_DOUBLE, local_data, local_data_count, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        free(data);
    }
    else {
        MPI_Bcast(bin_counts, n, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatter(data, local_data_count, MPI_DOUBLE, local_data, local_data_count, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);
} /* generate_data */

void print_data(int my_rank, int local_data_count, int data_count, int n, double local_data[], int bin_counts[]) {
    double* data = NULL;
    int i;

    if(my_rank == 0) {
        data = malloc(data_count * sizeof(double));
        
        MPI_Gather(local_data, local_data_count, MPI_DOUBLE, data, local_data_count, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        printf("Data: ");
        for(i = 0; i < data_count; i++) {
            printf("%0.3lf, ", data[i]);
        }
    }
    else {
        MPI_Gather(local_data, local_data_count, MPI_DOUBLE, data, local_data_count, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
} /* print_data */

int find_bin(double target, int n, double bin_width, double lower_bound) {
    int i;

    for(i = 0; i < n; i++) {
        if(target >= (lower_bound + (bin_width * i)) && target < (lower_bound + (bin_width * (i + 1)))) {
            return i;
        }
    }

    return -1;
} /* find_bin */

void update_bin_counts(int local_data_count, int n, double lower_bound, double local_data[], int bin_counts[], double bin_width) {
    int* local_bin_counts = NULL;
    int i, bin;
    
    local_bin_counts = malloc(n * sizeof(double));
    
    /* Initialize local bin array */
    for(i = 0; i < n; i++) {
        local_bin_counts[i] = 0;
    }

    for(i = 0; i < local_data_count; i++) {
        /* Update local_bin_counts on the basis of its assigned elements */
        bin = find_bin(local_data[i], n, bin_width, lower_bound);
        local_bin_counts[bin]++;
    }

    for(int i = 0; i < n; i++) {
        MPI_Reduce(&local_bin_counts[i], &bin_counts[i], 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    }
}