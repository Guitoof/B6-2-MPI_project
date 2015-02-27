#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include "config.h"

void print( int *M, int size );
void printErrors(int *M, int *check, int size);
void randomInit(int *M, int range, int size);
void transpose(int *M, int size);
void product(int *A, int *B, int *C, int size);

#endif
