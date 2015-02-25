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

  int *done;

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
  done = (int*)malloc(nbProcs*sizeof(int));

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

    /* Send data to other process */
    // Rows of A & columns of B
    for (n = 0; n < nbProcs; ++n)
    {
      MPI_Issend(&A[n*blockSize*SIZE], SIZE * blockSize, MPI_INT, n, TAG, MPI_COMM_WORLD, &sendRequests[n]);
      MPI_Issend(&B[n*blockSize*SIZE], SIZE * blockSize, MPI_INT, n, TAG, MPI_COMM_WORLD, &sendRequests[n]);
    }

  }
  else
  {
    // Receive data
    MPI_Irecv(&rowBlock, SIZE * blockSize, MPI_INT, 0, TAG, MPI_COMM_WORLD, &recvRequests[n]);
    MPI_Irecv(&colBlock, SIZE * blockSize, MPI_INT, 0, TAG, MPI_COMM_WORLD, &recvRequests[n]);

    // Process and display
    block[rank] = 0;
    for (k = 0; k < SIZE; ++k){
      block[rank] += rowBlock[0] * colBlock[0];
    }
    //printf("C%d%d: %d \n",rank, rank, block[rank]);
    //printf("\n");
  }

  if (rank == 0)
  {
    printf("\n\nSolution recherchée :\n");
    print(check);
  }

  MPI_Finalize();
  return EXIT_SUCCESS;
}
