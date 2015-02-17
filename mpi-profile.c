#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <mpi.h>

const int DIM = 10000;
const int MAX_INT = 10000;
const int ITER = 100;
const int REPS = 10;
const int BUFF_SIZE = 1000000000;

int getRandomInt (int max)
{
  return rand() % max;
}

uint64_t dotProduct (int * a, int * b)
{
  int len = sizeof(a) / sizeof(a[0]);
  int lenB = sizeof(b) / sizeof(b[0]);
  assert(len == lenB);

  uint64_t sum = 0;
  int i;
  for (i=0; i<len; i++) {
    sum += a[i] * b[i];
  }

  return sum;
}

void populateVectors (int * a, int * b)
{
  int i;
  for (i=0; i<DIM; i++) {
    a[i] = getRandomInt(MAX_INT);
    b[i] = getRandomInt(MAX_INT);
  }
}

void calcRandomDotProducts (int reps)
{
  int a[DIM], b[DIM];

  int i;
  clock_t start = clock();
  for (i=0; i<reps; i++) {
    populateVectors(a, b);
    dotProduct(a, b);
  }
  clock_t end = clock();
  printf("\tFinished %d iterations in %0.6f ms\n",
    reps,
    1000 * (end-start)/(double)CLOCKS_PER_SEC
    );
}

void clock_buffer_transfer(char * send_buff, char * recv_buff, int buff_size,
 int world_rank, int send_partner, int recv_partner)
{
  double starttime, endtime, recvtime;
  printf("Sending %0.4f MB (%d bytes) #%d --> #%d\n", 
    (double)buff_size / 1000000, buff_size, world_rank, send_partner);

  starttime = MPI_Wtime();

  if (world_rank != 0) {

    MPI_Send(send_buff, buff_size, MPI_CHAR, send_partner, 
      1, MPI_COMM_WORLD);
    recvtime = MPI_Wtime();
    MPI_Recv(recv_buff, buff_size, MPI_CHAR, recv_partner, 
      1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  } else {

    recvtime = MPI_Wtime();
    MPI_Recv(recv_buff, buff_size, MPI_CHAR, recv_partner, 
      1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Send(send_buff, buff_size, MPI_CHAR, send_partner, 
      1, MPI_COMM_WORLD);

  }

  endtime = MPI_Wtime();

  // data transfer time = latency + message size / bandwidth
  printf("Received #%d <-- #%d in %0.4f ms \n", 
      world_rank, send_partner, (endtime-starttime) * 1000);
}

void pass_buffer(int world_size, int world_rank, int buff_size)
{
  // Pair each process with a sender and recipient
  int send_partner, recv_partner;
  send_partner = (world_rank + 1) % world_size;
  recv_partner = (world_rank == 0) ? world_size - 1 : world_rank - 1;

  // Allocate the buffers
  char *send_buff = (char *) malloc(buff_size * sizeof(char));
  char *recv_buff = (char *) malloc(buff_size * sizeof(char));

  int scale[4] = {
    0, 1000, 1000000, 1000000000
  };
  int i;
  for (i=0; i<=4; i++) {
    clock_buffer_transfer(
      send_buff, recv_buff, scale[i],
      world_rank, send_partner, recv_partner
    );
  }

  free(send_buff);
  free(recv_buff);
}

int main (int argc, char **argv)
{
  
  int world_size, world_rank;
  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  pass_buffer(world_size, world_rank, BUFF_SIZE);

  MPI_Finalize();

  return EXIT_SUCCESS;
}
