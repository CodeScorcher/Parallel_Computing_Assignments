#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void Read_n(int* n_p, int* local_n_p, int my_rank, int comm_sz, MPI_Comm comm);
void Check_for_error(int local_ok, char fname[], char message[], MPI_Comm comm);
void Read_data(double local_vec1[], double local_vec2[], double* scalar_p, int local_n, int my_rank, int comm_sz, MPI_Comm comm);
void Print_vector(double local_vec[], int local_n, int n, char title[], int my_rank, MPI_Comm comm);
double Par_dot_product(double local_vec1[], double local_vec2[], int local_n, MPI_Comm comm);
void Par_vector_scalar_mult(double local_vec[], double scalar, double local_result[], int local_n);

int main(void) {
   int n, local_n;
   double *local_vec1, *local_vec2;
   double scalar;
   double *local_scalar_mult1, *local_scalar_mult2;
   double dot_product;
   int comm_size, my_rank;
   
   // Intialize the MPI Environment
   MPI_Init(NULL, NULL);
   MPI_Comm comm = MPI_COMM_WORLD;
   
   // Determine size of group associated with our communicator.
   MPI_Comm_size(comm, &comm_size);

   // Determine rank of the calling process in our communicator.
   MPI_Comm_rank(comm, &my_rank);

   /* Print input data */
   // n: size of vectors
   // Scalar: Number from user input
   Read_n(&n, &local_n, my_rank, comm_size, comm);
   Read_data(local_vec1, local_vec2, &scalar, local_n, my_rank, comm_size, comm);

   /* Print vector results */
   Print_vector(local_vec1, local_n, n, "Vector 1:", my_rank, comm);
   Print_vector(local_vec2, local_n, n, "Vector 2:", my_rank, comm);
   
   /* Compute and print dot product */
   // Vector 1: a1 a2 ... an-1
   // Vector 2: b1 b2 ... bn-1
   dot_product = Par_dot_product(local_vec1, local_vec2, local_n, comm);

   if(my_rank == 0) {
      printf("Scalar: %.2f\n", scalar);
      printf("Dot Product:%.2f\n", dot_product);
   }

   /* Compute scalar multiplication and print out result */
   // Scalar Mult Vec 1: scalar(a1 a2 ... an-1)
   // Scalar Mult Vec 2: scalar(b1 b2 ... bn-1)
   // Par_vector_scalar_mult(local_vec1, scalar, local_scalar_mult1, local_n);
   // Par_vector_scalar_mult(local_vec2, scalar, local_scalar_mult2, local_n);

   // Print_vector(local_scalar_mult1, local_n, n, "Scalar Mult Vec 1:", my_rank, comm);
   // Print_vector(local_scalar_mult2, local_n, n, "Scalar Mult Vec 2:", my_rank, comm);

   // free(local_scalar_mult2);
   // free(local_scalar_mult1);
   // free(local_vec2);
   // free(local_vec1);

   MPI_Finalize();
   return 0;
}

/*-------------------------------------------------------------------*/
void Check_for_error(
            int      local_ok  /* in */, 
            char     fname[]   /* in */, 
            char     message[] /* in */, 
            MPI_Comm comm      /* in */) {
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
}  /* Check_for_error */


/* Get the input of n: size of the vectors, and then calculate local_n according to comm_sz and n */
/* where local_n is the number of elements each process obtains */
/*-------------------------------------------------------------------*/
void Read_n(int* n_p, int* local_n_p, int my_rank, int comm_size, MPI_Comm comm) {
   if (my_rank == 0) {
      printf("What is the size of the vector?\n");
      scanf("%d", n_p);

      *local_n_p = (*n_p / comm_size);

      MPI_Bcast(local_n_p, 1, MPI_DOUBLE, 0, comm);
   }
   else {
      MPI_Bcast(local_n_p, 1, MPI_DOUBLE, 0, comm);
   }
}  /* Read_n */


/* local_vec1 and local_vec2 are the two local vectors of size local_n which the process pertains */
/* process 0 will take the input of the scalar, the two vectors a and b */
/* process 0 will scatter the two vectors a and b across all processes */
/*-------------------------------------------------------------------*/
void Read_data(double local_vec1[], double local_vec2[], double* scalar_p, int local_n, int my_rank, int comm_size, MPI_Comm comm) {
   double* a = NULL;
   double* b = NULL;
   int i;
   
   MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   
   if (my_rank == 0) {
      printf("What is the scalar?\n");
      scanf("%lf", scalar_p);
      MPI_Bcast(scalar_p, 1, MPI_DOUBLE, 0, comm);
   }
   
   if (my_rank == 0) {
      a = malloc(local_n * comm_size * sizeof(double));
      b = malloc(local_n * comm_size * sizeof(double));

      printf("Enter the first vector:\n");
      for (i = 0; i < local_n * comm_size; i++)
         scanf("%lf", &a[i]);
      
      MPI_Scatter(a, local_n, MPI_DOUBLE, local_vec1, local_n, MPI_DOUBLE, 0, comm);
      free(a);
      
      printf("Enter the second vector:\n");
      for (i = 0; i < local_n * comm_size; i++)
         scanf("%lf", &b[i]);
      
      MPI_Scatter(b, local_n, MPI_DOUBLE, local_vec2, local_n, MPI_DOUBLE, 0, comm);
      free(b);
   }
   else {
      MPI_Bcast(scalar_p, 1, MPI_DOUBLE, 0, comm);
      MPI_Scatter(a, local_n, MPI_DOUBLE, local_vec1, local_n, MPI_DOUBLE, 0, comm);
      MPI_Scatter(b, local_n, MPI_DOUBLE, local_vec2, local_n, MPI_DOUBLE, 0, comm);
   }
}  /* Read_data */

/* The print_vector gathers the local vectors from all processes and print the gathered vector */
/*-------------------------------------------------------------------*/
void Print_vector(double local_vec[], int local_n, int n, char title[], int my_rank, MPI_Comm comm) {
   double* a = NULL;
   int i;
   
   if (my_rank == 0) {
      a = malloc(n * sizeof(double));
      
      MPI_Gather(local_vec, local_n, MPI_DOUBLE, a, local_n, MPI_DOUBLE, 0, comm);
      
      printf("%s\n", title);
      for(i = 0; i < n; i++)
         printf("%.2f ", a[i]);
      printf("\n");

      free(a);
   }
   else {
      MPI_Gather(local_vec, local_n, MPI_DOUBLE, a, local_n, MPI_DOUBLE, 0, comm);
   }
}  /* Print_vector */


/* This function computes and returns the partial dot product of local_vec1 and local_vec2 */
/*-------------------------------------------------------------------*/
double Par_dot_product(double local_vec1[], double local_vec2[], int local_n, MPI_Comm comm) {
   int local_i;
   double dot_product = 0;
   double local_dot_product = 0;
   
   for(local_i = 0; local_i < local_n; local_i++) {
      local_dot_product = local_dot_product + local_vec1[local_i] * local_vec2[local_i];
   }

   MPI_Reduce(&local_dot_product, &dot_product, 1, MPI_DOUBLE, MPI_SUM, 0, comm);
   
   return dot_product;
}  /* Par_dot_product */


/* This function gets the vector which is the scalar times local_vec, and put the vector into local_result */
/*-------------------------------------------------------------------*/
void Par_vector_scalar_mult(double local_vec[], double scalar, double local_result[], int local_n) {
   int local_i;

   for(local_i = 0; local_i < local_n; local_i++) {
      local_result[local_i] = local_vec[local_i] * scalar;
   }
}  /* Par_vector_scalar_mult */