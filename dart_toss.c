#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

void read_tosses(int my_rank, int comm_size, long long int* tosses, long long int* local_tosses);
void monte_carlo(long long int local_tosses, long long int *local_number_in_circle);
void print_result(int my_rank, long long int tosses, long long int total_number_in_circle, long long int local_number_in_circle, double* pi_estimate);

int main(void) {
    int my_rank, comm_size;
    long long int tosses, local_tosses, total_number_in_circle, local_number_in_circle;
    double pi_estimate;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    local_number_in_circle = 0;
    total_number_in_circle = 0;

    read_tosses(my_rank, comm_size, &tosses, &local_tosses);
    monte_carlo(local_tosses, &local_number_in_circle);
        
    print_result(my_rank, tosses, total_number_in_circle, local_number_in_circle, &pi_estimate);

    MPI_Finalize();
    return 0;
}

void read_tosses(int my_rank, int comm_size, long long int* tosses, long long int* local_tosses) {
    if (my_rank == 0) {
      printf("What are the number of tosses?\n");
      scanf("%lld", tosses);

      *local_tosses = (*tosses / comm_size);

      MPI_Bcast(local_tosses, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
   }
   else {
      MPI_Bcast(local_tosses, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
   }
}

void monte_carlo(long long int local_tosses, long long int *local_number_in_circle) {
    long long int toss;
    double x, y, distance_squared;
    
    srand(time(NULL));

    for(toss = 0; toss < local_tosses; toss++) {
        x = (((double)rand() * (2 - (0)))/(double)RAND_MAX + (0)) - 1;
        y = (((double)rand() * (2 - (0)))/(double)RAND_MAX + (0)) - 1;

        distance_squared = x*x + y*y;

        if(distance_squared <= 1) {
            (*local_number_in_circle)++;
        }
    }
}

void print_result(int my_rank, long long int tosses, long long int total_number_in_circle, long long int local_number_in_circle, double* pi_estimate) {
    MPI_Reduce(&local_number_in_circle, &total_number_in_circle, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        *pi_estimate = (4.0*(double)total_number_in_circle)/((double) tosses);
        printf("Result: %lf", *pi_estimate);
    }
}