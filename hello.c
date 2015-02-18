#include <stdio.h>
#include <mpi.h>

void printSizeOfArray(int *a)
{
	int size = sizeof(a) / sizeof(a[0]);
	printf("Size of array is %d\n", size);
}

int main (int argc, char **argv)
{

  int a1[10];
  int a2[100];
  int a3[5000];
  int *a4 = (int*)malloc(64 * sizeof(int));

  printSizeOfArray(a1);
  printSizeOfArray(a2);
  printSizeOfArray(a3);
  printSizeOfArray(a4);
  
  int sz, myid;

  MPI_Init (&argc, &argv);

  MPI_Comm_size (MPI_COMM_WORLD, &sz);
  
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  printf ("Hello, I am %d of %d processors!\n", myid, sz);

  MPI_Finalize ();

  


  exit (0);
}



