#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "time.h"
#include "matrixutils.h"
#include "config.h"
#include "compute.h"
#include "argsparser.h"

void routine(Config config, int rank, int printMonoprocessor)
{
  // Matrices
  int *A, *B, *C, *check;

  double monoProcTime;

  A = (int*)malloc(config.size * config.size * sizeof(int));
  B = (int*)malloc(config.size * config.size * sizeof(int));
  C = (int*)malloc(config.size * config.size * sizeof(int));
  check = (int*)malloc(config.size * config.size * sizeof(int));

  config.blockSize = config.size / config.nbProcs;

  if (rank == 0)
  {
    // Initialize matrices
    randomInit(A, RANGEMAX, config.size);
    if (config.verbose)
    {
      printf("A = \n");
      print(A, config.size);
    }
    randomInit(B, RANGEMAX, config.size);
    if (config.verbose)
    {
      printf("B = \n");
      print(B, config.size);
    }

    monoProcTime = MPI_Wtime();
    // Compute matricial product : A*B = check for further verification
    product(A, B, check, config.size);
    monoProcTime = MPI_Wtime() - monoProcTime;
    /* Print monoprocess computation time */
    if (printMonoprocessor)
    {
      printf("_____________________________________\n");
      printf("Temps de calcul monoprocesseur : %f sec\n", monoProcTime);
      printf("_____________________________________\n");
    }
  }

  double elapsedTime;

  if (config.algorithm == NONBLOCKING)
    computeAsynchronously(config, rank, A, B, C, monoProcTime);
  else
    computeSynchronously(config, rank, A, B, C, monoProcTime);

  if (rank == 0)
  {
    if (config.verbose)
    {
      printf("\nRésultat cherché :\n");
      print(check, config.size);
      printf("\nRésultat calculé :\n");
      print(C, config.size);
      printf("\nErreurs :\n");
      printErrors(C, check, config.size);
    }
  }

  free(A);
  free(B);
  free(C);
  free(check);
}

int main (int argc, char** argv)
{
  srand(time(NULL));

  /* Variables declaration*/
  // MPI
  int rank;

  Config config;

  /* MPI Initialization */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &config.nbProcs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  parseArguments(argc, argv, &config, rank);

  if (rank == 0)
  {
    printf("Programme de multiplication matricielle parallèlisé sous MPI\n");
    printf("Auteurs : Woody Rousseau & Guillaume Diallo-Mulliez\n\n");
  }

  // Check if the number of processes is a divider of the matrices' size
  if (config.size % config.nbProcs)
  {
    printf("La taille des matrices multipliées (%d) doit être un multiple du nombre de processeurs utilisé (%d) !\n", config.size, config.nbProcs);
    exit(EXIT_FAILURE);
  }
  if (config.benchmark)
  {
    int sizes[3] = {512, 1024, 2048};
    int i;
    for (i = 0; i < 3; i++)
    {
      config.size = sizes[i];
      config.algorithm = BLOCKING;
      if (rank == 0)
      {
        printf("===============\n");
        printf("N = %d\n", config.size);
        printf("===============\n");
      }
      routine(config, rank, 1);
      config.algorithm = NONBLOCKING;
      routine(config, rank, 0);
      if (rank == 0)
      {
        printf("\n\n");
      }
    }
  }
  else
  {
    routine(config, rank, 1);
  }

  MPI_Finalize();
  return EXIT_SUCCESS;
}
