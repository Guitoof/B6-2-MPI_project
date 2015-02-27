#include "compute.h"
#include "matrixutils.h"
#include "mpi.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void benchmark(double monoProcTime, double elapsedTime, Config config, int rank)
{
  int n;
  MPI_Status status;

  double *procTimes = (double *)malloc(config.nbProcs*sizeof(double));

  MPI_Send( &elapsedTime, 1, MPI_DOUBLE, 0, TAG, MPI_COMM_WORLD );
  if (rank == 0)
  {
    /* Receive processing times from each process */
    procTimes[0] = elapsedTime;
    for (n = 0; n < config.nbProcs; ++n)
      if (n!=rank)
        MPI_Recv( &procTimes[n], 1, MPI_DOUBLE, n, TAG, MPI_COMM_WORLD, &status );

    /* Benchmark parallelism gain */

    double longestTime = 0.0;
    double rawGain = 0.0;
    double gainRate = 0.0;
    for (n = 0; n < config.nbProcs; ++n)
    {
      if ( procTimes[n] > longestTime )
        longestTime = procTimes[n];
    }
    printf("Pire temps processeur : %f sec\n", longestTime);
    rawGain = monoProcTime - longestTime;
    printf("Gain Absolu / Monoprocesseur : %f sec\n", rawGain);
    gainRate = 100 * rawGain / monoProcTime;
    printf("Gain Relatif / Monoprocesseur : %f %%\n", gainRate);
    printf("_____________________________________\n");
  }

  free(procTimes);
}

void computeSynchronously(Config config, int rank, int *A, int *B, int *C, double monoProcTime)
{
  if (rank==0)
    printf("Algorithme bloquant avec N=%d : \n\n", config.size);

  int n, i, j, k;
  double beginTime, elapsedTime;
  // Block data buffers
  int *rowBlock, *colBlock, *block;

  MPI_Status status;
  MPI_Request *sendRequests, *recvRequests;

  rowBlock = (int*)malloc(config.blockSize * config.size * sizeof(int));
  colBlock = (int*)malloc(config.blockSize * config.size * sizeof(int));
  block = (int*)malloc(config.blockSize * config.size * sizeof(int));

  // Scatter A's block columns between processes
  MPI_Scatter(A, config.size * config.blockSize, MPI_INT, rowBlock, config.size * config.blockSize, MPI_INT, 0, MPI_COMM_WORLD);
  // Scatter B's block rows between processes
  if (rank == 0)
  {
    transpose(B, config.size);
  }
  MPI_Scatter(B, config.size * config.blockSize, MPI_INT, colBlock, config.size * config.blockSize, MPI_INT, 0, MPI_COMM_WORLD);

  beginTime = MPI_Wtime();

  /* Calcul des produits */
  for (n = 0; n < config.nbProcs; n++)
  {
    /* Le proc 0 transmet les nbLignes lignes suivantes de A */
    if (rank == 0)
    {
      memcpy(colBlock, &(B[config.size * config.blockSize * n]), config.size * config.blockSize * sizeof(int));
    }
    MPI_Bcast(colBlock, config.size*config.blockSize, MPI_INT, 0, MPI_COMM_WORLD);

    /* Chaque proc calcul le bloc (iProc,rang) */
    for (j = 0; j < config.blockSize; j++)
    {
      for (i = 0; i < config.blockSize; i++)
      {
        int index = config.blockSize*n + config.size*i + j;
        block[index] = 0;
        for (k = 0; k < config.size; k++)
        {
          block[index] += rowBlock[config.size*i + k] * colBlock[config.size*j + k];
        }
      }
    }
  }

  elapsedTime = MPI_Wtime() - beginTime;

  /* Gather computed blocks into the result matrix C */
  MPI_Gather( block, config.size*config.blockSize, MPI_INT, C, config.size*config.blockSize, MPI_INT, 0, MPI_COMM_WORLD );

  if (rank == 0)
  {
    //transpose(C, config.size);
  }

  free(rowBlock);
  free(colBlock);
  free(block);

  benchmark(monoProcTime, elapsedTime, config, rank);
}


void computeAsynchronously(Config config, int rank, int *A, int *B, int *C, double monoProcTime)
{
  if (rank==0)
    printf("Algorithme non-bloquant avec N=%d\n\n", config.size);

  int n, i, j, k;
  double beginTime, elapsedTime;

  // Block data buffers
  int *rowBlock, *colBlock, *block;

  MPI_Status status;
  MPI_Request *sendRequests, *recvRequests;

  sendRequests = (MPI_Request*) malloc(config.nbProcs*sizeof(MPI_Request));
  recvRequests = (MPI_Request*) malloc(config.nbProcs*sizeof(MPI_Request));
  rowBlock = (int*) malloc(config.blockSize * config.size * sizeof(int));
  colBlock = (int*) malloc(config.blockSize * config.size * sizeof(int));
  block = (int*) malloc(config.blockSize * config.size * sizeof(int));

  // Scatter A's block columns between processes
  MPI_Scatter(A, config.size * config.blockSize, MPI_INT, rowBlock, config.size * config.blockSize, MPI_INT, 0, MPI_COMM_WORLD);
  // Scatter B's block rows between processes
  if (rank == 0)
  {
    transpose(B, config.size);
  }
  MPI_Scatter(B, config.size * config.blockSize, MPI_INT, colBlock, config.size * config.blockSize, MPI_INT, 0, MPI_COMM_WORLD);

  beginTime = MPI_Wtime();

  /*
  * Each process :
  *   sends its col block from B to each other processes
  *   receives each col block from every other processes
  *   computes the corresponding column block
  */
  for (n = 0; n < config.nbProcs; ++n)
  {
    if (n != rank)
    {
      MPI_Issend(colBlock, config.size*config.blockSize, MPI_INT, n, TAG, MPI_COMM_WORLD, &sendRequests[n]);
      MPI_Irecv(&B[config.size*config.blockSize*n], config.size*config.blockSize, MPI_INT, n, TAG, MPI_COMM_WORLD, &recvRequests[n]);
    }
  }

  // Compute diagonal blocks (No need for the asynchronous communications to be completed)
  for (j = 0; j < config.blockSize; ++j)
  {
    for (i = 0; i < config.blockSize; ++i)
    {
      int index = config.blockSize*rank + config.size*i + j;
      block[index] = 0;
      for (k = 0; k < config.size; ++k)
      {
        block[index] += rowBlock[config.size*i + k] * colBlock[config.size*j + k];
      }
    }
  }

  for (n = 0; n < config.nbProcs; ++n)
  {
    if (n != rank)
    {
      MPI_Wait(&recvRequests[n], &status);
      for (j = 0; j < config.blockSize; ++j)
      {
        for (i = 0; i < config.blockSize; ++i)
        {
          int index = config.blockSize*n + config.size*i + j;
          block[index] = 0;
          for (k = 0; k < config.size; ++k)
          {
            block[index] += rowBlock[config.size*i+k]*B[config.size*(config.blockSize*n+j)+k];
          }
        }
      }
    }
  }

  elapsedTime = MPI_Wtime() - beginTime;

  /* Gather computed blocks into the result matrix C */
  MPI_Gather( block, config.size*config.blockSize, MPI_INT, C, config.size*config.blockSize, MPI_INT, 0, MPI_COMM_WORLD );

  free(rowBlock);
  free(colBlock);
  free(block);
  free(sendRequests);
  free(recvRequests);

  benchmark(monoProcTime, elapsedTime, config, rank);
}
