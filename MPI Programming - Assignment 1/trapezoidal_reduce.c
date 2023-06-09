#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void Read_data(int my_rank, double* a_p, double* b_p, int* n_p);
double Trap(double left_endpoint, double right_endpoint, int trap_count, double base_len);
double f(double num);

int main(void) {
    int my_rank, comm_sz, n , local_n;
    double a, b, h, local_a, local_b;
    double local_int, total_int;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    /* Input is as follows:
        a - lower bound of the integral
        b - upper bound of the integral
        n - number of subintervals (trapezoids) */
    Read_data(my_rank, &a, &b, &n);

    /* h is the same for all processes */
    h = (b - a)/n;

    /* So is the number of trapezoids */
    local_n = n/comm_sz;

    local_a = a + my_rank * local_n * h;
    local_b = local_a + local_n * h;
    local_int = Trap(local_a, local_b, local_n, h);
    
    /* Use MPI_Reduce in order to compute the total number of subintervals */
    MPI_Reduce(&local_int, &total_int, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (my_rank == 0) {
        printf("With n = %d trapezoids, our estimate\n", n);
        printf("of the integral from %f to %f = %.15e\n", a, b, total_int);
    }

    MPI_Finalize();
    return 0;
} /* main */

void Read_data(int my_rank, double* a_p, double* b_p, int* n_p) {
    if(my_rank == 0) {
        printf("Enter a, b, and n:\n");
        scanf("%lf %lf %d", a_p, b_p, n_p);

        MPI_Bcast(a_p, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(b_p, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(n_p, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    else {
        MPI_Bcast(a_p, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(b_p, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(n_p, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
}

double f(double num) {
    return num * 1;
}

double Trap(double left_endpoint, double right_endpoint, int trap_count, double base_len) {
    double estimate, x;
    int i;

    estimate = (f(left_endpoint) + f(right_endpoint)) / 2.0;
    
    for(i = 1; i <= trap_count-1; i++) {
        x = left_endpoint + i * base_len;
        estimate += f(x);
    }
    
    estimate = estimate * base_len;

    return estimate;
}
