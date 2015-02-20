#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <mpi.h>

void pair_processes(int id, int *partner, bool *is_sender, int world_size)
{
  // Pair each process with its neighbor
  assert(world_size % 2 == 0);
  if (id % 2 == 0) {
    *is_sender = true;
    *partner = id + 1;
  } else {
    is_sender = false;
    *partner = id - 1;
  }
  // printf("Process #%d is paired with process #%d\n", id, *partner);
}

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
  int sz, myid;

  MPI_Init(&argc, &argv);

  MPI_Comm_size (MPI_COMM_WORLD, &sz);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  int testIterations;
  parse_cli_args(argc, argv, myid, &testIterations);

  int partner;
  bool is_sender;
  pair_processes(myid, &partner, &is_sender, sz);

  double starttime, endtime, total_time;
  MPI_Status status;
  int count = testIterations;
  char val = 'a';

  MPI_Barrier(MPI_COMM_WORLD);
  while (count > 0) {
    if (is_sender) {
      count--;
      // printf("Process #%d sending %d\n", myid, count);
      starttime = MPI_Wtime();
      MPI_Send(&val, 1, MPI_CHAR, partner, 1, MPI_COMM_WORLD);
      MPI_Recv(&val, 1, MPI_CHAR, partner, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      endtime = MPI_Wtime();
      total_time += (endtime-starttime);
    } else {
      MPI_Recv(&val, 1, MPI_CHAR, partner, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      MPI_Send(&val, 1, MPI_CHAR, partner, 1, MPI_COMM_WORLD);
    }
  }

  if (is_sender) {
    double average_latency = total_time / testIterations / 2;
    printf("Average latency between process %d and %d after %d messages is %0.6f µs\n", 
      myid, partner, testIterations, average_latency * 1000000);
  }
  
  MPI_Finalize();
}