#ifndef CONFIG_H
#define CONFIG_H

// MPI
#define TAG 100

typedef enum
{
  BLOCKING,
  NONBLOCKING
} Algorithm;

typedef struct {
  int nbProcs;
  int blockSize;
  int verbose;
  int size;
  int benchmark;
  Algorithm algorithm;
} Config;

#endif
