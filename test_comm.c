#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#define PROFILE_TAG         1
#define BYES_TO_MEGABYTES   1000000
#define TRANSFER_TOTAL      10000000000
#define EXTRA_SMALL_PACKET  100
#define SMALL_PACKET        1000
#define MEDIUM_PACKET       1000000
#define LARGE_PACKET        1000000000
#define BYTES_IN_BUFFER     1000000000

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

  // Latency

  int testIterations;
  parse_cli_args(argc, argv, myid, &testIterations);

  // Declare profiling variables and helpers
  int partner;
  bool is_sender;
  MPI_Status status;
  double starttime, endtime, recvtime, totaltime;

  // Pair each process with its neighbor
  assert(sz % 2 == 0);
  if (myid % 2 == 0) {
    partner = myid + 1;
    is_sender = true;
  } else {
    partner = myid - 1;
    is_sender = false;
  }

  if (myid == 0) {
    printf("\n#%d: LATENCY TEST.\n", myid);
  }
  if (is_sender) {
   printf("#%d: Sending %d total %d byte messages between %d and %d\n", 
    myid, testIterations, 1, myid, partner);
  }
    
  MPI_Barrier(MPI_COMM_WORLD);
  int count;
  for (count = testIterations; count > 0; count--) {
    if (is_sender) {
      starttime = MPI_Wtime();
      MPI_Send(&starttime, 1, MPI_DOUBLE, partner, 
        PROFILE_TAG, MPI_COMM_WORLD);
    } else {
      MPI_Recv(&recvtime, 1, MPI_DOUBLE, partner, 
        PROFILE_TAG, MPI_COMM_WORLD, &status);
      endtime = MPI_Wtime();
      totaltime += endtime - recvtime;
    }
  }

  if (!is_sender) {
    double average_latency = totaltime / testIterations;
    printf("\t#%d: Average latency between process %d and %d after %d messages is %0.6f µs\n", 
      myid, partner, myid, testIterations, average_latency * 1000000);
  }

  
  MPI_Barrier(MPI_COMM_WORLD);

  // Create the 
  char  *buff = (char *)  malloc(BYTES_IN_BUFFER * sizeof(char));
  double duration = 10.0;

  if (myid == 0) {
   printf("\n#%d: BANDWIDTH TEST.\n", myid);
  }
  
  int message_count = 0;
  const int len = 4;
  int packet_sizes[len] = {
    EXTRA_SMALL_PACKET,
    SMALL_PACKET,
    MEDIUM_PACKET,
    LARGE_PACKET
  };

  int i, iter_count;
  unsigned long long j;
  for (i=0; i<len; i++) {
    if (is_sender) {
      printf("#%d: Sending %ld total MB between %d and %d. ", 
        myid,  TRANSFER_TOTAL / BYES_TO_MEGABYTES, myid, partner);
      printf("Packet size %d bytes.\n", packet_sizes[i]);
    }   

    MPI_Barrier(MPI_COMM_WORLD);
    starttime = MPI_Wtime();

    iter_count = 0;
    for (j=0; j<TRANSFER_TOTAL; j+=packet_sizes[i]) {
      if (is_sender) {
        MPI_Send(buff, packet_sizes[i], MPI_CHAR, partner, PROFILE_TAG, 
          MPI_COMM_WORLD);
      } else {
        MPI_Recv(buff, packet_sizes[i], MPI_CHAR, partner, PROFILE_TAG, 
          MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_CHAR, &message_count);
        assert(message_count == packet_sizes[i]);
      }
      iter_count++;
    } // end inner for

    endtime = MPI_Wtime();
    assert(iter_count == TRANSFER_TOTAL / packet_sizes[i]);

    if (!is_sender) {
      printf("\t#%d: Time elapsed: %0.8f seconds. Megabytes transferred: %0.8f\n", 
        myid, 
        endtime - starttime, 
        (double) TRANSFER_TOTAL / BYES_TO_MEGABYTES);
      printf("\t#%d: %0.5f Megabytes / second\n",
        myid, 
        (double) TRANSFER_TOTAL / (endtime-starttime) / BYES_TO_MEGABYTES);
    }

    MPI_Barrier(MPI_COMM_WORLD);

  } // end outer for

  free(buff);

  MPI_Finalize();

  return EXIT_SUCCESS;
}
