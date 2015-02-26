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
  double beginTime, elapsedTime;

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

  // Check if the number of processes is a divider of the matrices' SIZE
  if (SIZE % nbProcs)
  {
    printf("La taille des matrices multipliées (%d) doit être un multiple du nombre de processeurs utilisé (%d) !\n", SIZE, nbProcs);
    exit(EXIT_FAILURE);
  }

  blockSize = SIZE / nbProcs;
  rowBlock = (int*)malloc(blockSize * SIZE * sizeof(int));
  colBlock = (int*)malloc(blockSize * SIZE * sizeof(int));
  block = (int*)malloc(blockSize * blockSize * sizeof(int));

  if (rank == 0)
  {
    // Initialize matrices
    randomInit(A, 10);
    #ifdef VERBOSE
    printf("A = \n");
    print(A);
    #endif
    randomInit(B, 10);
    #ifdef VERBOSE
    printf("B = \n");
    print(B);
    #endif

    // Compute matricial product : A*B = check for further verification
    product(A, B, check);
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
  *   sends its row block from A to each other processes
  *   receives each row block from every other processes
  *   computes the corresponding column block
  */
  for (n = 0; n < nbProcs; ++n)
  {
    if (n != rank)
    {
      MPI_Issend(rowBlock, SIZE*blockSize, MPI_INT, n, TAG, MPI_COMM_WORLD, &sendRequests[n]);
      MPI_Irecv(&A[SIZE*blockSize*n], SIZE*blockSize, MPI_INT, n, TAG, MPI_COMM_WORLD, &recvRequests[n]);
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
      for (i = 0; i < blockSize; ++i)
      {
        for (j = 0; j < blockSize; ++j)
        {
          block[blockSize*n + SIZE*j + i] = 0;
          for (k = 0; k < SIZE; ++k)
          {
            block[blockSize*n + SIZE*j + i] += A[SIZE*(blockSize*n + i) + k] * colBlock[SIZE*j + k];
          }
        }
      }
    }
  }

  elapsedTime = MPI_Wtime() - beginTime;
  printf("Temps écoulé pour le processeur %d : %f sec\n", rank, elapsedTime);

  /* Gather computed blocks into the result matrix C */
  MPI_Gather( block, SIZE*blockSize, MPI_INT, C, SIZE*blockSize, MPI_INT, 0, MPI_COMM_WORLD );

  transpose(C);

  if (rank == 0)
  {
    printf("\nRésultat cherché :\n");
    print(check);
    printf("\nRésultat calculé :\n");
    print(C);
    printf("\n\nErreurs :\n");
    printErrors(C, check);
  }

  free(rowBlock);
  free(colBlock);
  free(block);
  free(sendRequests);
  free(recvRequests);

  MPI_Finalize();
  return EXIT_SUCCESS;
}
