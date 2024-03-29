/* donde.c
 * A hybrid MPI / OpenMP program that reports the CPU where each thread
 * of each rank is executing. Used to assist in determining correct 
 * binding behavior.
 * Rory Kelly
 * 3 May 2017
 * --
 * Added support for heterogenrous numbers of threads/rank
 * 3 Nov 2022
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <mpi.h>
#include <omp.h>

int main(int argc, char **argv){
  int mpi_id;             // MPI Task ID
  int n_mpi;              // Number of MPI Tasks
  int omp_id;             // OpenMP Thread ID
  int n_omp;              // Number of OpenMP threads
  int total_omp;          // Sum of # threads across all ranks
  int my_cpu;             // CPU # where task/thread is executing
  int mpi_tsup_lev;       // provided level of MPI thread support   

  int *thrds_for_rank;    // array w/ # threads for each rank, used buy rank 0
  char *thrd_str;         // per-thread output strings
  char *all_strs;         // per-thread strings for all processes
  char node_name[MPI_MAX_PROCESSOR_NAME]; // the node where process / thread is executing
  int length;             // length of returned string
  int *offsets;           // offsets for the MPIGatherv call
  int *recvcts;           // receive counts for the MPIGatherv call

  // initialize MPI and get information about the numbers of ranks,
  // threads, and execution hosts
  MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &mpi_tsup_lev);
  MPI_Comm_size(MPI_COMM_WORLD, &n_mpi);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_id);
  MPI_Get_processor_name(node_name, &length);
  n_omp = atoi(getenv("OMP_NUM_THREADS"));
  thrds_for_rank = (int*) malloc(n_mpi * sizeof(int));
  MPI_Gather(&n_omp, 1, MPI_INT, thrds_for_rank, 1, MPI_INT, 0, MPI_COMM_WORLD); 
  MPI_Reduce(&n_omp, &total_omp, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  // allocate space to hold a string from each thread for this rank
  thrd_str = (char*) malloc(n_omp * MPI_MAX_PROCESSOR_NAME * sizeof(char));

  // allocate space on rank 0 to hold all strings from all threads
  // of all ranks. Calculate necessary offsets and receive counts
  // to gather the strings to rank 0
  if (mpi_id == 0){
     printf("Total threads: %d\n",total_omp);
     for (int i=0; i<n_mpi; i++){
	     printf("  Rank %d has %d threads\n",i,thrds_for_rank[i]);
     }
     printf("--- --- --- --- ---\n");
     all_strs = (char*) malloc(total_omp * MPI_MAX_PROCESSOR_NAME * sizeof(char));
     offsets = (int*) malloc(n_mpi * sizeof(int));
     recvcts = (int*) malloc(n_mpi * sizeof(int));
     offsets[0] = 0;
     recvcts[0] = thrds_for_rank[0]*MPI_MAX_PROCESSOR_NAME;
     for(int i=1; i<n_mpi; i++){
	offsets[i] = offsets[i-1]+thrds_for_rank[i-1]*MPI_MAX_PROCESSOR_NAME;
        recvcts[i] = thrds_for_rank[i]*MPI_MAX_PROCESSOR_NAME;
     }
  }

  // Collect MPI Rank, OpenMP thread, and execution CPU on host
  #pragma omp parallel private(omp_id, n_omp, my_cpu) shared(thrd_str)
  {
     omp_id = omp_get_thread_num();
     n_omp = omp_get_num_threads();
     my_cpu = sched_getcpu();

     if (omp_id == 0){
        sprintf(&thrd_str[MPI_MAX_PROCESSOR_NAME * omp_id], "MPI Task %3d, OpenMP thread %d of %d (cpu %d of %s)", mpi_id, omp_id, n_omp, my_cpu, node_name);
     } else {
        sprintf(&thrd_str[MPI_MAX_PROCESSOR_NAME * omp_id], "              OpenMP thread %d of %d (cpu %d of %s)", omp_id, n_omp, my_cpu, node_name);
     }
  }

  // gather to rank 0 and print in order
  MPI_Gatherv(thrd_str, n_omp*MPI_MAX_PROCESSOR_NAME, MPI_CHAR, all_strs, recvcts, offsets, MPI_CHAR, 0, MPI_COMM_WORLD);
  if (mpi_id == 0){
	for(int j=0; j<total_omp; j++){
	   puts(&all_strs[j*MPI_MAX_PROCESSOR_NAME]);
        }
  } 
  return MPI_Finalize();
}
