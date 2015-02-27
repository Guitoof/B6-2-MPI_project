#include "matrixutils.h"

#include <stdio.h>
#include <stdlib.h>

void print( int *M, int size )
{
  int i, j;

  for (i = 0; i < size; ++i){
    for (j = 0; j < size; ++j){
      printf(" %d ", M[i*size + j]);
    }
    printf("\n\r");
  }
  printf("\n\r");
}

void printErrors(int *M, int *check, int size)
{
  int i, j;
  for (i = 0; i < size; i++)
  {
    for (j = 0; j < size; j++)
    {
      if (M[i * size + j] != check[i * size + j])
      {
        printf("x ");
      }
      else
      {
        printf("o ");
      }
    }
    printf("\n");
  }
}

void randomInit(int *M, int range, int size)
{
  int i, j;
  for (i = 0; i < size; ++i)
  {
    for (j = 0; j < size; ++j)
    {
      M[size*i + j]  = rand() % range;
    }
  }
}

void transpose(int *M, int size)
{
  int i, j;
  for (i = 0; i < size; i++)
  {
    for (j = 0; j < i; j++)
    {
      M[size*i + j] += M[size*j + i];
      M[size*j + i] = M[size*i + j] - M[size*j + i];
      M[size*i + j] -= M[size*j + i];
    }
  }
}

void product(int *A, int *B, int *C, int size)
{
  int i, j, k;
  for (i = 0; i < size; i++)
  {
    for (j = 0; j < size; j++)
    {
      C[size*i + j] = 0;
      for (k = 0; k < size; k++)
      {
        C[size*i + j] += A[size*i + k] * B[size*k + j];
      }
    }
  }
}
