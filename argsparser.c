#include "argsparser.h"
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

void display_usage( int rank )
{
  if (rank == 0)
  {
    puts( "MPI Multiplicateur Matrices - v0.1 - Guillaume Diallo-Mulliez & Woody Rousseau" );
    puts( "Utilisations :");
    puts( "mpiexec -n 4 MPI_project.x --N 512 --algorithm blocking");
    puts( "mpiexec -n 4 MPI_project.x --N 1024 --algorithm nonblocking");
    puts( "mpiexec -n 4 MPI_project.x --benchmark");
  }
  exit( EXIT_FAILURE );
}

void parseArguments(int argc, char **argv, Config *config, int rank)
{
  static const char *optString = "vbs:a:h?";

  static const struct option longOpts[] = {
    { "verbose", no_argument, NULL, 'v' },
    { "benchmark", no_argument, NULL, 'b' },
    { "N", required_argument, 0, 'N'},
    { "algorithm", required_argument, 0, 'a'},
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
  };

  int longIndex = 0, opt = 0;
  config->size = 0;
  config->verbose = 0;
  config->benchmark = 0;

  opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
  while( opt != -1 ) {
    switch( opt ) {
      case 'v':
      config->verbose = 1; /* true */
      break;

      case 'b':
      config->benchmark = 1;
      break;

      case 'N':
      config->size = atoi(optarg);
      break;

      case 'a':
      if(!strcmp(optarg, "nonblocking"))
        config->algorithm = NONBLOCKING;
      else
        config->algorithm = BLOCKING;
      break;

      case 'h':   /* fall-through is intentional */
      case '?':
      display_usage(rank);
      break;

      default:
      /* You won't actually get here. */
      break;
    }

    opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
  }

  if (config->size == 0 && config->benchmark == 0)
  {
    if (rank == 0)
      printf("L'argument --N n'a pas été fourni. Utilise N = 512\n");
    config->size = 512;
  }
  if (config->algorithm != BLOCKING && config->algorithm != NONBLOCKING)
  {
    if (rank == 0)
      printf("L'argument --algorithm n'a pas été fourni (blocking ou nonblocking). blocking est utilisé par défaut.\n");
    config->algorithm = BLOCKING;
  }
  if (config->verbose && (config->size > 64 || config->benchmark))
  {
    if (rank == 0)
      printf("Le flag --verbose est inutile pour des matrices trop larges\n");
    config->verbose = 0;
  }
}
