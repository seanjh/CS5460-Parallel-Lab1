#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
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

  // Latency

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

  if (is_sender) {
   printf("#%d: LATENCY TEST.\n", myid);
   printf("#%d: Sending %d total %d byte messages between %d and %d\n", myid, testIterations, 1, myid, partner);
  }

  double starttime, endtime, total_time;
  MPI_Status status;
  
  char inVal, outVal = 'a';
    
  MPI_Barrier(MPI_COMM_WORLD);
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

  if (is_sender) {
    double average_latency = total_time / testIterations / 2;
    printf("\t#%d: Average latency between process %d and %d after %d messages is %0.6f µs\n", 
      myid, myid, partner, testIterations, average_latency * 1000000);
  }

  
  MPI_Barrier(MPI_COMM_WORLD);

  // Bandwidth
  char  *out_buff = (char *)  malloc(1000000000 * sizeof(char));
  char  *in_buff  = (char *)  malloc(1000000000 * sizeof(char)); 

  // send packets continuously for 10 seconds
  double duration = 10.0;
  int bytes_transferred = 0;
  int iter = 0;

  if (is_sender) {
   printf("#%d: BANDWIDTH TEST.\n", myid);
  }
  
  int packet_sizes[4] = {1, 1000, 1000000, 1000000000};
  int i;
  for (i=0; i<4; i++) {
    if (is_sender) {
      printf("#%d: Sending packets between %d and %d for %0.1f seconds. ", myid, myid, partner, duration);
      printf("Packet size %d bytes.\n", packet_sizes[i]);
    }   

    MPI_Barrier(MPI_COMM_WORLD);
    starttime = MPI_Wtime();
    endtime = MPI_Wtime();
    MPI_Barrier(MPI_COMM_WORLD);
    if (endtime < starttime) {
      printf("\t#%d: Invalid MPI_Wtime entries\n", myid);
      exit(EXIT_FAILURE);
    }

    while (endtime - starttime < duration) {
      // endtime = MPI_Wtime();
      if (is_sender) {
        MPI_Send(out_buff, packet_sizes[i], MPI_CHAR, partner, tag, MPI_COMM_WORLD);
      } else {
        MPI_Recv(in_buff, packet_sizes[i], MPI_CHAR, partner, tag, MPI_COMM_WORLD, &status);
        bytes_transferred += packet_sizes[i];
      }
      endtime = MPI_Wtime();
      iter++;
      // printf("\t#%d: Starttime is %f, Endtime is %f. Difference is %f\n", myid, starttime, endtime, endtime-starttime);
      // printf("\t\t#%d: %0.10fs elapsed \n", myid, endtime - starttime);  
    }

    if (!is_sender) {
      printf("\t#%d: completed after %d iterations.\n", 
        myid, iter);
      printf("\t#%d: Time elapsed: %0.8f seconds. Bytes transferred: %d\n", 
        myid, endtime - starttime, bytes_transferred);
      printf("\t#%d: %0.5f bytes / second\n",
        myid, bytes_transferred / (endtime - starttime));
    }
  }

  free(out_buff);
  free(in_buff);

  // printf("\t#%d: completed after %d iterations. Time elapsed: %0.8f seconds. Bytes transferred: %d\n", 
  //   myid, iter, endtime - starttime, bytes_transferred);
  // printf("\t#%d: finished\n", myid);


  MPI_Finalize();

  return EXIT_SUCCESS;
}