#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "time.h"

#define TRUE 1
#define FALSE 0

#define TAG 100

#define SIZE 4

#define VERBOSE     // Print DEBUG messages


void print( int matrix[SIZE*SIZE] )
{
  int i,j;

  for (i = 0; i < SIZE; ++i){
    for (j = 0; j < SIZE; ++j){
      printf(" %d ", matrix[i*SIZE + j]);
    }
    printf("\n\r");
  }
  printf("\n\r");
}

void initMatrices(int A[SIZE*SIZE], int B[SIZE*SIZE])
{
  int i, j;
  srand(time(NULL));

  printf("Initialisation des matrices A, B de taille %dx%d\n", SIZE, SIZE);

  for (i = 0; i < SIZE; ++i){
    for (j = 0; j < SIZE; ++j){
      A[i*SIZE + j]  = rand();
      B[i*SIZE + j] = rand();
    }
  }

  #ifdef VERBOSE

  printf("A = \n");
  print(A);
  printf("B = \n");
  print(B);

  #endif

}

int main (int argc, char* argv[])
{
  /* Variables declaration*/
  // MPI
  int nbProcs, rank;
  MPI_Status status;
  MPI_Request* sendRequests;
  MPI_Request* recvRequests;

  int *done;

  // Matrices
  int A[SIZE*SIZE], B[SIZE*SIZE], C[SIZE*SIZE];
  int blockSize;

  // Block data buffers
  int *rowBlock, *colBlock, *block;

  // Iterations
  int i,j, k, n;


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
    initMatrices(A,B);

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
      printf("C%d%d: %d \n",rank, rank, block[rank]);
      printf("\n");
    }

  MPI_Finalize();
  return EXIT_SUCCESS;
}
