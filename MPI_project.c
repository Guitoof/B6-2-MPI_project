#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "time.h"
#include "matrixutils.h"
#include "config.h"

int main (int argc, char** argv)
{
  srand(time(NULL));

  /* Variables declaration*/
  // MPI
  int nbProcs, rank;
  MPI_Status status;
  MPI_Request* sendRequests;
  MPI_Request* recvRequests;
  double beginTime, elapsedTime, monoProcTime;
  double *procTimes;

  // Matrices
  int A[SIZE*SIZE], B[SIZE*SIZE], C[SIZE*SIZE], check[SIZE*SIZE];
  int blockSize;

  // Block data buffers
  int *rowBlock, *colBlock, *block;

  // Iterations
  int i, j, k, n;


  /* MPI Initialization */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nbProcs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  sendRequests = (MPI_Request*) malloc(nbProcs*sizeof(MPI_Request));
  recvRequests = (MPI_Request*) malloc(nbProcs*sizeof(MPI_Request));
  procTimes = (double *)malloc(nbProcs*sizeof(double));


  // Check if the number of processes is a divider of the matrices' SIZE
  if (SIZE % nbProcs)
  {
    printf("La taille des matrices multipliées (%d) doit être un multiple du nombre de processeurs utilisé (%d) !\n", SIZE, nbProcs);
    exit(EXIT_FAILURE);
  }

  blockSize = SIZE / nbProcs;
  rowBlock = (int*)malloc(blockSize * SIZE * sizeof(int));
  colBlock = (int*)malloc(blockSize * SIZE * sizeof(int));
  block = (int*)malloc(blockSize * SIZE * sizeof(int));

  if (rank == 0)
  {
    // Initialize matrices
    randomInit(A, MAT_MAX);
    #ifdef VERBOSE
    printf("A = \n");
    print(A);
    #endif
    randomInit(B, MAT_MAX);
    #ifdef VERBOSE
    printf("B = \n");
    print(B);
    #endif

    // Compute matricial product : A*B = check for further verification
    monoProcTime = MPI_Wtime();
    product(A, B, check);
    monoProcTime = MPI_Wtime() - monoProcTime;
    /* Print monoprocess computation time */
    printf("Temps de calcul monoprocesseur : %f sec\n", monoProcTime);
  }

  // Scatter A's block columns between processes
  MPI_Scatter(A, SIZE * blockSize, MPI_INT, rowBlock, SIZE * blockSize, MPI_INT, 0, MPI_COMM_WORLD);
  // Scatter B's block rows between processes
  if (rank == 0)
  {
    transpose(B);
  }
  MPI_Scatter(B, SIZE * blockSize, MPI_INT, colBlock, SIZE * blockSize, MPI_INT, 0, MPI_COMM_WORLD);

  beginTime = MPI_Wtime();


  /*
  * Each process :
  *   sends its col block from B to each other processes
  *   receives each col block from every other processes
  *   computes the corresponding column block
  */
  for (n = 0; n < nbProcs; ++n)
  {
    if (n != rank)
    {
      MPI_Issend(colBlock, SIZE*blockSize, MPI_INT, n, TAG, MPI_COMM_WORLD, &sendRequests[n]);
      MPI_Irecv(&B[SIZE*blockSize*n], SIZE*blockSize, MPI_INT, n, TAG, MPI_COMM_WORLD, &recvRequests[n]);
    }
  }

  // Compute diagonal blocks (No need for the asynchronous communications to be completed)
  for (i = 0; i < blockSize; ++i)
  {
    for (j = 0; j < blockSize; ++j)
    {
      block[blockSize*rank + SIZE*j + i] = 0;
      for (k = 0; k < SIZE; ++k)
      {
        block[blockSize*rank + SIZE*j + i] += rowBlock[SIZE*i + k] * colBlock[SIZE*j + k];
      }
    }
  }

  for (n = 0; n < nbProcs; ++n)
  {
    if (n != rank)
    {
      MPI_Wait(&recvRequests[n], &status);
      for (j = 0; j < blockSize; ++j)
      {
        for (i = 0; i < blockSize; ++i)
        {
          block[blockSize*n + SIZE*j + i] = 0;
          for (k = 0; k < SIZE; ++k)
          {
            block[blockSize*n + SIZE*j + i] += rowBlock[SIZE*i + k] * B[SIZE*(blockSize*n + j) + k];
          }
        }
      }
    }
  }

  /* Print multipleprocess computation times */
  elapsedTime = MPI_Wtime() - beginTime;
  printf("Temps écoulé pour le processeur %d : %f sec\n", rank, elapsedTime);

  #ifdef BENCHMARKS
  /* Each process sends its processing time to the root process so that it can benchmark them */
  MPI_Send( &elapsedTime, 1, MPI_DOUBLE, 0, TAG, MPI_COMM_WORLD );
  if (rank == 0)
  {
    /* Receive processing times from each process */
    procTimes[0] = elapsedTime;
    for (n = 0; n < nbProcs; ++n)
      if (n!=rank)
        MPI_Recv( &procTimes[n], 1, MPI_DOUBLE, n, TAG, MPI_COMM_WORLD, &status );

    /* Benchmark parallelism gain */
    printf("\n=========================================================================");
    printf("\nRésultats du benchmark associé à la parallèlisation de l'algorithme :\n\n");
    printf("_____________________________________\n");

    double longestTime = 0.0;
    double rawGain = 0.0;
    double gainRate = 0.0;
    for (n = 0; n < nbProcs; ++n)
    {
      if ( procTimes[n] > longestTime )
        longestTime = procTimes[n];
    }
    rawGain = monoProcTime - longestTime;
    printf("Optimisation brute : %f sec\n", rawGain);
    gainRate = 100 * rawGain / monoProcTime;
    printf("Taux d'optimisation : %f %%\n", gainRate);

    printf("=========================================================================\n");
  }
  #endif


  /* Gather computed blocks into the result matrix C */
  MPI_Gather( block, SIZE*blockSize, MPI_INT, C, SIZE*blockSize, MPI_INT, 0, MPI_COMM_WORLD );


  if (rank == 0)
  {
    #ifdef VERBOSE
    printf("\n=========================================================================\n");
    printf("\nRésultat du calcul matriciel et comparaison au résultat attendu:\n\n");
    printf("_____________________________________\n");

    printf("\nRésultat cherché :\n");
    print(check);
    printf("\nRésultat calculé :\n");
    print(C);
    printf("\n\nErreurs :\n");
    printErrors(C, check);

    printf("\n=========================================================================\n");
    #endif
  }

  free(rowBlock);
  free(colBlock);
  free(block);
  free(sendRequests);
  free(recvRequests);
  free(procTimes);

  MPI_Finalize();
  return EXIT_SUCCESS;
}
