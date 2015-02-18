#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>


// MPI_Recv(recv_buff, buff_size, MPI_CHAR, MPI_ANY_SOURCE, 
//       MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

// int MPI_Send(const void* buf, int count, MPI_Datatype datatype, int dest,
//                   int tag, MPI_Comm comm)


// typedef struct DotProductDescriptionStruct {
//    int a_len;
//    int b_len;
//    double vectorBuffer[];
// } DotProductDescription;

double dotProduct(double *a, double *b, int len)
{
  int i;
  double result = 0.0;
  for (i=0; i<len; i++)
    result += a[i]*b[i];
  return result;
}


void workerTask(int id, int maxLen)
{
  int i;
  double *a; 
  int a_len;
  MPI_Status a_status;
  double *b;
  int b_len;
  MPI_Status b_status;

  double partialResult=0.0;

  a = (double *)malloc(maxLen * sizeof(double));
  b = (double *)malloc(maxLen * sizeof(double));
  MPI_Recv(a, maxLen, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &a_status);
  MPI_Recv(b, maxLen, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &b_status);

  MPI_Get_count(&a_status, MPI_DOUBLE, &a_len);
  MPI_Get_count(&b_status, MPI_DOUBLE, &b_len);

  assert(a_len == b_len);

  partialResult=dotProduct(a, b, a_len);

  free(a);
  free(b);

  //printf ("Hello, I am %d. My partial result is %f.\n", id, partialResult);

  MPI_Send(&partialResult, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
}

#define BUFF_SIZE 1024
int main (int argc, char **argv)
{
  
  int sz, myid, ai;
  int testLen, testIterations;

  MPI_Init(&argc, &argv);

  MPI_Comm_size (MPI_COMM_WORLD, &sz);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (argc != 3)
  {
    if(myid==0)
      fprintf(stderr, "Usage ./test length iterations\n");

    exit(1);
  }

  testLen = atoi(argv[1]);
  testIterations = atoi(argv[2]);
  //printf("testLen=%d\n", testLen);

  int j;
  for(j=0;j<testIterations;j++)
  {
  
    //compute parallel results

    //first, generate test vectors

    //next, distribute
    if (myid == 0)
    {

      int i;
      double *a = (double *)malloc(testLen * sizeof(double));
      double *b = (double *)malloc(testLen * sizeof(double));

      srand((unsigned)time(NULL));

      for(i=0; i<testLen; i++)
      {
        a[i]=((double)rand()/(double)RAND_MAX);
        b[i]=((double)rand()/(double)RAND_MAX);
      }

      double result;

      double starttime, endtime;

      starttime = MPI_Wtime();
      result=dotProduct(a, b, testLen);
      endtime = MPI_Wtime();
      printf("Single result: %f\n", result);
      printf("Computed in %0.4f ms \n", (endtime-starttime) * 1000);

      //distribute
      starttime = MPI_Wtime();

      assert(testLen % sz == 0);
      int partitionLen = testLen/sz;
      for(i=1; i<sz; i++)
      {
        double *a_src = a + (i*partitionLen);
        double *b_src = b + (i*partitionLen);

        //printf("Sending data to %d\n", i);
        MPI_Send(a_src, partitionLen, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
        //printf("Sending data to %d\n", i);
        MPI_Send(b_src, partitionLen, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
      }

      //compute partial result for master
      double partialResult;

      partialResult=dotProduct(a, b, partitionLen);
      free(a);
      free(b);m

      //printf ("Hello, I am %d. My partial result is %f.\n", myid, partialResult);

      //combine results
      result=partialResult;
      for(i=1; i<sz; i++)
      {
        MPI_Recv(&partialResult, 1, MPI_DOUBLE, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        result += partialResult;
      }

      endtime = MPI_Wtime();

      printf ("Distributed results: %f\n", result);
      printf("Computed in %0.4f ms \n", (endtime-starttime) * 1000);

      sleep(2);
    } 

    //receive data and compute partial results
    else 
    {
      assert(testLen % sz == 0);
      int partitionLen = testLen/sz;
      workerTask(myid, partitionLen);
    }
  }

  MPI_Finalize();

  return EXIT_SUCCESS;
}