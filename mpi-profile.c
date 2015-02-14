#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <mpi.h>

const int DIM = 10000;
const int MAX_INT = 10000;
const int ITER = 10;
const int REPS = 10;

int getRandomInt (int max)
{
  return rand() % max;
}

uint64_t dotProduct (int * a, int * b)
{
  int len = sizeof(a) / sizeof(a[0]);
  int lenB = sizeof(b) / sizeof(b[0]);
  assert(len == lenB);

  uint64_t sum = 0;
  int i;
  for (i=0; i<len; i++) {
    sum += a[i] * b[i];
  }

  return sum;
}

void populateVectors (int * a, int * b)
{
  int i;
  for (i=0; i<DIM; i++) {
    a[i] = getRandomInt(MAX_INT);
    b[i] = getRandomInt(MAX_INT);
  }
}

void calcRandomDotProducts (int reps)
{
  int a[DIM], b[DIM];

  int i;
  clock_t start = clock();
  for (i=0; i<reps; i++) {
    populateVectors(a, b);
    dotProduct(a, b);
  }
  clock_t end = clock();
  printf("\tFinished %d iterations in %0.6f ms\n",
    reps,
    1000 * (end-start)/(double)CLOCKS_PER_SEC
    );
}

int main (int argc, char **argv)
{
  // Message latency
  // Message bandwidth
  // Computation time per operation (compute a large dot product, and divide by the number of floating point operations)
  int i;
  for (i=0; i<ITER; i++) {
    calcRandomDotProducts(REPS);
  }

  return EXIT_SUCCESS;
}
