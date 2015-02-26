#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

#include "config.h"

void print( int M[SIZE*SIZE] );
void printErrors(int M[SIZE*SIZE], int check[SIZE*SIZE]);
void randomInit(int M[SIZE*SIZE], int range);
void transpose(int M[SIZE*SIZE]);
void product(int A[SIZE*SIZE], int B[SIZE*SIZE], int C[SIZE*SIZE]);

#endif
