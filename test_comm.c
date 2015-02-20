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

#define ITERATIONS  1000
int main (int argc, char **argv)
{
  int sz, myid;

  MPI_Init(&argc, &argv);

  MPI_Comm_size (MPI_COMM_WORLD, &sz);
  MPI_Comm_rank (MPI_COMM_WORLD, &myid);

  int partner;
  bool is_sender;
  pair_processes(myid, &partner, &is_sender, sz);
  

  double starttime, endtime, total_time;
  MPI_Status status;
  int count = ITERATIONS;

  MPI_Barrier(MPI_COMM_WORLD);

  while (count > 0) {
    if (is_sender) {
      count--;
      // printf("Process #%d sending %d\n", myid, count);
      starttime = MPI_Wtime();
      MPI_Send(&count, 1, MPI_INT, partner, 1, MPI_COMM_WORLD);
      MPI_Recv(&count, 1, MPI_INT, partner, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      endtime = MPI_Wtime();
      total_time += (endtime-starttime);
    } else {
      MPI_Recv(&count, 1, MPI_INT, partner, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      MPI_Send(&count, 1, MPI_INT, partner, 1, MPI_COMM_WORLD);
    }
  }

  if (is_sender) {
    double average_latency = total_time / ITERATIONS / 2;
    printf("Average latency between process %d and %d after %d messages is %0.6f µs\n", 
      myid, partner, ITERATIONS, average_latency * 1000000);
  }

  MPI_Finalize();
}