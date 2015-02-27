#ifndef COMPUTE_H
#define COMPUTE_H

#include "config.h"

void computeSynchronously(Config config, int rank, int *A, int *B, int *C, double monoProcTime);
void computeAsynchronously(Config config, int rank, int *A, int *B, int *C, double monoProcTime);

#endif
