#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define ERR_BUFF_SIZE 1024
int main (int argc, char **argv)
{
  
  int sz, myid, ai;

  const int testLen = atoi(argv[1]); 

  MPI_Init(&argc, &argv);

  MPI_Comm_size (MPI_COMM_WORLD, &sz);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  double *a = (double *)malloc(testLen * sizeof(double));

  srand((unsigned)time(NULL));



  if(myid == 0)
  {
    int i;
  	for(i=0; i<testLen; i++)
    {
      a[i]=((double)rand()/(double)RAND_MAX);
    }

    MPI_Send(a, testLen, MPI_DOUBLE, 1, 1, MPI_COMM_WORLD);

  }

  else
  {
    int a_len;
    MPI_Status a_status;
    char err[ERR_BUFF_SIZE];
    int r;
    MPI_Recv(a, testLen, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &a_status);

    memset(err, 0, ERR_BUFF_SIZE);
    
    MPI_Error_string(a_status.MPI_ERROR, err, &r);
    printf("a status: %d: %d: %s\n",a_status.MPI_ERROR, r, err);

    MPI_Get_count(&a_status, MPI_DOUBLE, &a_len);

    printf("a length: %d\n",a_len);
  }

  free(a);

  MPI_Finalize();

  return EXIT_SUCCESS;
}
