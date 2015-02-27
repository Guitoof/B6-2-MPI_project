#ifndef COMPUTE_H
#define COMPUTE_H

#include "config.h"

double computeSynchronously(Config config, int rank, int *A, int *B, int *C, double monoProcTime);
double computeAsynchronously(Config config, int rank, int *A, int *B, int *C, double monoProcTime);

#endif
