#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <mpi.h>

const int DIM = 100;
const int MAX_INT = 10000;
const int ITER = 1000;

int getRandomInt (int max)
{
  return rand() % max;
}

int dotProduct (int * a, int * b)
{
  int len = sizeof(a) / sizeof(a[0]);
  int lenB = sizeof(b) / sizeof(b[0]);
  assert(len == lenB);

  int sum = 0;
  int i;
  for (i=0; i<len; i++) {
    sum += a[i] * b[i];
  }

  return sum;
}

void calcRandomDotProducts ()
{
  int a[DIM], b[DIM];

  int i;
  for (i=0; i<DIM; i++) {
    a[i] = getRandomInt(MAX_INT);
    b[i] = getRandomInt(MAX_INT);
  }
  printf("Dot product: %d\n", dotProduct(a, b));
}

int main (int argc, char **argv)
{

  // Message latency
  // Message bandwidth
  // Computation time per operation (compute a large dot product, and divide by the number of floating point operations)
  int i;
  for (i=0; i<ITER; i++) {
    calcRandomDotProducts();
  }

  return EXIT_SUCCESS;
}
