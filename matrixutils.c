#include "matrixutils.h"

#include <stdio.h>
#include <stdlib.h>

void print( int M[SIZE*SIZE] )
{
  int i, j;

  for (i = 0; i < SIZE; ++i){
    for (j = 0; j < SIZE; ++j){
      printf(" %d ", M[i*SIZE + j]);
    }
    printf("\n\r");
  }
  printf("\n\r");
}

void randomInit(int M[SIZE*SIZE], int range)
{
  int i, j;
  for (i = 0; i < SIZE; ++i)
  {
    for (j = 0; j < SIZE; ++j)
    {
      M[i*SIZE + j]  = rand() % range;
    }
  }
}

void transpose(int M[SIZE*SIZE])
{
  int i, j;
  for (i = 0; i < SIZE; i++)
  {
    for (j = 0; j < i; j++)
    {
      M[i * SIZE + j] += M[j * SIZE + i];
      M[j * SIZE + i] = M[i * SIZE + j] - M[j * SIZE + i];
      M[i * SIZE + j] -= M[j * SIZE + i];
    }
  }
}

void product(int A[SIZE*SIZE], int B[SIZE*SIZE], int C[SIZE*SIZE])
{
  int i, j, k;
  for (i = 0; i < SIZE; i++)
  {
    for (j = 0; j < SIZE; j++)
    {
      C[i * SIZE + j] = 0;
      for (k = 0; k < SIZE; k++)
      {
        C[i * SIZE + j] += A[i * SIZE + k] * B[k * SIZE + j];
      }
    }
  }
}
