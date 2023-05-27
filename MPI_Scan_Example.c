#include <stdio.h>
#include <mpi.h>

void compute_prefix(int my_rank, int pdf, int cdf);

#define WCOMM MPI_COMM_WORLD
int main(int argc, char **argv){
    int my_rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(WCOMM, &my_rank);

    int pdf_i = 0;
    int cdf_i = 0;
    
    printf("%d", pdf_i);
    
    pdf_i = my_rank;

    compute_prefix(my_rank, pdf_i, cdf_i);

    // MPI_Scan(&pdf_i, &cdf_i, 1, MPI_INT, MPI_SUM, WCOMM);
    // printf("process %d: cumulative sum = %d\n", my_rank, cdf_i);

    MPI_Finalize();
}

void compute_prefix(int my_rank, int pdf, int cdf) {
    MPI_Scan(&pdf, &cdf, 1, MPI_INT, MPI_SUM, WCOMM);
    printf("process %d: cumulative sum = %d\n", my_rank, cdf);
}