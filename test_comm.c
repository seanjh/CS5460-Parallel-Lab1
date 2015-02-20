#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <mpi.h>

void parse_cli_args(int argc, char **argv, int myid, int *iter)
{
  if (argc != 2) {
    if (myid == 0) {
      fprintf(stderr, "Usage mpirun ./test_comm iterations\n");
    }
    exit(1);
  }

  *iter = atoi(argv[1]);
}

int main (int argc, char **argv)
{
  int sz, myid, tag;

  MPI_Init(&argc, &argv);

  MPI_Comm_size (MPI_COMM_WORLD, &sz);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);
  tag = 1;

  int testIterations;
  parse_cli_args(argc, argv, myid, &testIterations);

  int partner;
  bool is_sender;

  // Pair each process with its neighbor
  assert(sz % 2 == 0);
  if (myid % 2 == 0) {
    partner = myid + 1;
    is_sender = true;
  } else {
    partner = myid - 1;
    is_sender = false;
  }

  // if (is_sender) {
  //   printf("Process #%d is a sender\n", myid);
  // }

  double starttime, endtime, total_time;
  MPI_Status status;
  
  char inVal, outVal = 'a';
  int count;

  
  for (count = testIterations; count > 0; count--) {
    if (is_sender) {
      starttime = MPI_Wtime();
      MPI_Send(&outVal, 1, MPI_CHAR, partner, tag, MPI_COMM_WORLD);
      MPI_Recv(&inVal, 1, MPI_CHAR, partner, tag, MPI_COMM_WORLD, &status);
      endtime = MPI_Wtime();
      total_time += (endtime-starttime);
    } else {
      MPI_Recv(&inVal, 1, MPI_CHAR, partner, tag, MPI_COMM_WORLD, &status);
      MPI_Send(&outVal, 1, MPI_CHAR, partner, tag, MPI_COMM_WORLD);
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
  
  if (is_sender) {
    double average_latency = total_time / testIterations / 2;
    printf("Average latency between process %d and %d after %d messages is %0.6f µs\n", 
      myid, partner, testIterations, average_latency * 1000000);
  }

  MPI_Finalize();

  return EXIT_SUCCESS;
}