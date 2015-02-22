#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>
#include <string.h>
#include <math.h>


// MPI_Recv(recv_buff, buff_size, MPI_CHAR, MPI_ANY_SOURCE, 
//       MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

// int MPI_Send(const void* buf, int count, MPI_Datatype datatype, int dest,
//                   int tag, MPI_Comm comm)


// typedef struct DotProductDescriptionStruct {
//    int a_len;
//    int b_len;
//    double vectorBuffer[];
// } DotProductDescription;

typedef double (*seqOp)(double, double);
typedef double (*reduceOp)(double, double);
typedef double (*TestOperationType)(double *, double *, int);



#define ERR_BUFF_SIZE 1024

#define PRECISION 0.000001

double mult(double a, double b)
{
  return a*b;
}

double sum(double a, double b)
{
  return a+b;
}

double aggregate( double *a, 
                  double *b, 
                  int len, 
                  double (*seqOp)(double, double),
                  double (*reduceOp)(double, double))
{
  int i;
  double result = 0.0;
  for (i=0; i<len; i++)
    result = reduceOp(result, seqOp(a[i], b[i]));
  return result;
}

double powTestOp(double *a, double *b, int len)
{
  // int i;
  // double result = 0.0;
  // for (i=0; i<len; i++)
  //   result += a[i]*b[i];
  // return result;
  return aggregate(a, b, len, &pow, &sum);
  //return aggregate(a, b, len, &mult, &sum);
}

double dotProduct(double *a, double *b, int len)
{
  int i;
  double result = 0.0;
  for (i=0; i<len; i++)
    result += a[i]*b[i];
  return result;
}

TestOperationType testOperation;

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
  MPI_Recv(a, maxLen, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &a_status);
  MPI_Recv(b, maxLen, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD, &b_status);

  char err[ERR_BUFF_SIZE];
  int r;
  
  memset(err, 0, ERR_BUFF_SIZE);
  if(a_status.MPI_ERROR != MPI_SUCCESS)
  {
    MPI_Error_string(a_status.MPI_ERROR, err, &r);
    printf("a status: %d: %d: %s\n",a_status.MPI_ERROR, r, err);
  }

  memset(err, 0, ERR_BUFF_SIZE);

  if(b_status.MPI_ERROR != MPI_SUCCESS)
  {
    MPI_Error_string(b_status.MPI_ERROR, err, &r);
    printf("b status: %d: %d: %s\n",b_status.MPI_ERROR, r, err);
  }

  MPI_Get_count(&a_status, MPI_DOUBLE, &a_len);
  MPI_Get_count(&b_status, MPI_DOUBLE, &b_len);

  //printf("a length: %d\n",a_len);
  //printf("b length: %d\n",b_len);

  assert(maxLen == a_len);
  assert(a_len == b_len);



  partialResult=testOperation(a, b, a_len);

  free(a);
  free(b);

  // printf ("Hello, I am %d. My partial result is %f. My length was %d.\n", id, partialResult, a_len);

  MPI_Send(&partialResult, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
}

#define BUFF_SIZE 1024
int main (int argc, char **argv)
{
  
  int sz, myid, ai;

  MPI_Init(&argc, &argv);

  MPI_Comm_size (MPI_COMM_WORLD, &sz);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  if (argc != 4)
  {
    if(myid==0)
      fprintf(stderr, "Usage ./test length iterations [pow|dot]\n");

    exit(1);
  }

  const int testLen = atoi(argv[1]); 
  const int testIterations = atoi(argv[2]);
  if(strcmp(argv[3], "dot")==0)
  {
    testOperation = &dotProduct;
  }
  else if(strcmp(argv[3], "pow")==0)
  {
    testOperation = &powTestOp;
  }
  else
  {
    if(myid==0)
      fprintf(stderr, "Usage ./test length iterations [pow|dot]\n");

    exit(1);
  }

  //printf("testLen=%d\n", testLen);

  int j;

  srand((unsigned)time(NULL));

  //double localResult, distributedResult;
  double *localResults = (double *)malloc(testIterations * sizeof(double));
  double *localDurations = (double *)malloc(testIterations * sizeof(double));
  double *distributedResults = (double *)malloc(testIterations * sizeof(double));
  double *distributedDurations = (double *)malloc(testIterations * sizeof(double));

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

      

      for(i=0; i<testLen; i++)
      {
        a[i]=((double)rand()/(double)RAND_MAX);
        b[i]=((double)rand()/(double)RAND_MAX);
      }

      

      double starttime, endtime;
      printf(".");
      fflush(stdout);
      starttime = MPI_Wtime();
      localResults[j]=testOperation(a, b, testLen);
      endtime = MPI_Wtime();
      printf(".");
      fflush(stdout);
      //printf("Local result: %f\n", localResult);
      localDurations[j] = (endtime-starttime);
      //printf("Computed in %0.4f ms \n", (endtime-starttime) * 1000);

      //distribute
      starttime = MPI_Wtime();

      assert(testLen % sz == 0);
      int partitionLen = testLen/sz;
      for(i=1; i<sz; i++)
      {
        double *a_src = a + (i*partitionLen);
        double *b_src = b + (i*partitionLen);

        // printf("Sending %d doubles to %d\n", partitionLen, i);
        MPI_Send(a_src, partitionLen, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
        // printf("Sending %d doubles to %d\n", partitionLen, i);
        MPI_Send(b_src, partitionLen, MPI_DOUBLE, i, 2, MPI_COMM_WORLD);
      }

      //compute partial result for master
      double partialResult;

      partialResult=testOperation(a, b, partitionLen);


      // printf ("Hello, I am %d. My partial result is %f.\n", myid, partialResult);

      //combine results
      distributedResults[j]=partialResult;
      for(i=1; i<sz; i++)
      {
        MPI_Recv(&partialResult, 1, MPI_DOUBLE, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        distributedResults[j] += partialResult;
      }

      endtime = MPI_Wtime();

      distributedDurations[j] = (endtime-starttime);

      free(a);
      free(b);

      //printf ("Distributed result: %f\n", distributedResult);
      //printf("Computed in %0.4f ms \n", (endtime-starttime) * 1000);

      //sleep(2);
    } 

    //receive data and compute partial results
    else 
    {

      assert(testLen % sz == 0);
      int partitionLen = testLen/sz;
      //printf("Size: %d, TestLen: %d, PartitionLen: %d\n", sz, testLen, partitionLen);
      workerTask(myid, partitionLen);
    }
  }

  if(myid == 0)
  {
    printf("\n");
    double avgLocalDuration=0.0;
    double avgDistributedDuration=0.0;
    double avgSpeedup;
    int i;
    for(i=0; i<testIterations; i++)
    {
      avgLocalDuration+=localDurations[i];
      avgDistributedDuration+=distributedDurations[i];

      //check for correctness
      double difference = fabs(localResults[i]-distributedResults[i]);
      if(difference/localResults[i] > PRECISION)
      {
        if(myid==0)
          fprintf(stderr, "FAILURE: Results out of range!\n");

        exit(1);
      }
    }


    avgLocalDuration = avgLocalDuration / (double)testIterations;
    avgDistributedDuration = avgDistributedDuration / (double)testIterations;

    avgSpeedup = avgLocalDuration/avgDistributedDuration;
    printf("Processes=%d, Vector Size=%d, Computation=%s\n", sz, testLen, argv[3]);
    printf("Average Local Computation Duration: %f\n", avgLocalDuration);
    printf("Average Local FLOPs: %f\n", ((double)testLen*2.0)/avgLocalDuration);
    printf("Average Distributed Computation Duration: %f\n", avgDistributedDuration);
    printf("Average Distributed FLOPs: %f\n", ((double)testLen*2.0)/avgDistributedDuration);
    printf("Average Speedup: %f\n", avgSpeedup);


  }

  MPI_Finalize();

  return EXIT_SUCCESS;
}
