#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <mpi.h>


// MPI_Recv(recv_buff, buff_size, MPI_CHAR, MPI_ANY_SOURCE, 
//       MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

// typedef struct DotProductDescriptionStruct {
//    int a_len;
//    int b_len;
//    double vectorBuffer[];
// } DotProductDescription;

#define BUFF_SIZE 1024
int main (int argc, char **argv)
{
  
  int sz, myid, ai;
  int testLen;

  //double recv_A[BUFF_SIZE];
  //double recv_B[BUFF_SIZE];

  MPI_Init(&argc, &argv);

  MPI_Comm_size (MPI_COMM_WORLD, &sz);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (argc != 2)
  {
    if(myid==0)
      fprintf(stderr, "Usage ./test length\n");
    
    exit(1);
  }

  testLen = atoi(argv[1]);
  //printf("testLen=%d\n", testLen);

  

  // if(myid==0)
  // {
  //   printf("argc=%d\n", argc);
  //   for(ai=0; ai<argc; ai++)
  //     printf("%s\n", argv[ai]);

  // }

  if (myid == 0)
  {

    int i;
    //double a[BUFF_SIZE];
    double *a = (double *)malloc(testLen * sizeof(double));
    //double b[BUFF_SIZE];
    double *b = (double *)malloc(testLen * sizeof(double));

    double result0, result1;

    srand((unsigned)time(NULL));

    for(i=0; i<testLen; i++)
    {
      a[i]=((double)rand()/(double)RAND_MAX);
      b[i]=((double)rand()/(double)RAND_MAX);
    }

    MPI_Send(a, testLen, MPI_DOUBLE, 1, 1, MPI_COMM_WORLD);
    MPI_Send(b, testLen, MPI_DOUBLE, 1, 1, MPI_COMM_WORLD);

    MPI_Recv(&result1, 1, MPI_DOUBLE, 1, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    printf ("Result computed by 1: %f\n", result1);

    result0=0.0;
    for(i=0; i<testLen; i++)
      result0 += a[i]*b[i];

    printf ("Result computed by 0: %f\n", result0);


    // DotProductDescription desc;
    // desc.a_len = 16;
    // desc.b_len = 1024;
    // memcpy(desc.vectorBuffer, a, 16);
    // memcpy(desc.vectorBuffer, a, 16);
  } else if (myid == 1)
  {
    int i;
    //double a[BUFF_SIZE];
    double *a = (double *)malloc(testLen * sizeof(double));
    int a_len;
    MPI_Status a_status;
    //double b[BUFF_SIZE];
    double *b = (double *)malloc(testLen * sizeof(double));
    int b_len;
    MPI_Status b_status;

    double result=0.0;

    MPI_Recv(a, testLen, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &a_status);
    MPI_Recv(b, testLen, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &b_status);

    MPI_Get_count(&a_status, MPI_DOUBLE, &a_len);
    MPI_Get_count(&b_status, MPI_DOUBLE, &b_len);

    assert(a_len == b_len);

    for(i=0; i<a_len; i++)
      result += a[i]*b[i];

    MPI_Send(&result, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
  }

  //printf ("Hello, I am %d of %d processors!\n", myid, sz);

  MPI_Finalize();

  return EXIT_SUCCESS;
}