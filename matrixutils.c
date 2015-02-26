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

void printErrors(int M[SIZE*SIZE], int check[SIZE*SIZE])
{
  int i, j;
  for (i = 0; i < SIZE; i++)
  {
    for (j = 0; j < SIZE; j++)
    {
      if (M[i * SIZE + j] != check[i * SIZE + j])
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

void randomInit(int M[SIZE*SIZE], int range)
{
  int i, j;
  for (i = 0; i < SIZE; ++i)
  {
    for (j = 0; j < SIZE; ++j)
    {
      M[SIZE*i + j]  = rand() % range;
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
      M[SIZE*i + j] += M[SIZE*j + i];
      M[SIZE*j + i] = M[SIZE*i + j] - M[SIZE*j + i];
      M[SIZE*i + j] -= M[SIZE*j + i];
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
      C[SIZE*i + j] = 0;
      for (k = 0; k < SIZE; k++)
      {
        C[SIZE*i + j] += A[SIZE*i + k] * B[SIZE*k + j];
      }
    }
  }
}
