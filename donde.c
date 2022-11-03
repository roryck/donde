/* donde.c
 * A hybrid MPI / OpenMP program that reports the CPU where each thread
 * of each rank is executing. Used to assist in determining correct 
 * binding behavior.
 * Rory Kelly
 * 3 May 2017
 */
#define _GNU_SOURCE
#define STRLEN 80
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <mpi.h>
#include <omp.h>

int main(int argc, char **argv){
  int mpi_id;         // MPI Task ID
  int n_mpi;          // Number of MPI Tasks
  int omp_id;         // OpenMP Thread ID
  int n_omp;          // Number of OpenMP threads
  int my_cpu;         // CPU # where task/thread is executing
  int mpi_tsup_lev;   // provided level of MPI thread support   

  char *thrd_str;         // per-thread output strings
  char *all_strs;         // per-thread strings for all processes
  char node_name[STRLEN]; // The node where process / thread is executing
  int length;             // Length of returned string
  int *offsets;
  int *recvcts;

  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &mpi_tsup_lev);
  MPI_Comm_size(MPI_COMM_WORLD, &n_mpi);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_id);
  MPI_Get_processor_name(node_name, &length);
  n_omp = atoi(getenv("OMP_NUM_THREADS"));

  thrd_str = malloc(n_omp * STRLEN * sizeof(char));
  if (mpi_id == 0){
     all_strs = malloc(n_mpi * n_omp * STRLEN * sizeof(char));
     offsets = (int*) malloc(n_mpi * sizeof(int));
     recvcts = (int*) malloc(n_mpi * sizeof(int)); 
     for(int i=0; i<n_mpi; i++){
        offsets[i] = i*n_omp*STRLEN;
	     recvcts[i] = n_omp*STRLEN;
     }
  }


  // Collect MPI Rank and OpenMP thread info
  #pragma omp parallel private(omp_id, n_omp, my_cpu) shared(thrd_str)
  {
     omp_id = omp_get_thread_num();
     n_omp = omp_get_num_threads();
     my_cpu = sched_getcpu();

     if (omp_id == 0){
        sprintf(&thrd_str[STRLEN * omp_id], "MPI Task %3d, OpenMP thread %d of %d (cpu %d of %s)", mpi_id, omp_id, n_omp, my_cpu, node_name);
     } else {
        sprintf(&thrd_str[STRLEN * omp_id], "              OpenMP thread %d of %d (cpu %d of %s)", omp_id, n_omp, my_cpu, node_name);
     }
  }

  // gather to rank 0 and print in order
  MPI_Gatherv(thrd_str, n_omp*STRLEN, MPI_CHAR, all_strs, recvcts, offsets, MPI_CHAR, 0, MPI_COMM_WORLD);
  if (mpi_id == 0){
        for(int j=0; j<n_mpi*n_omp; j++){
	   puts(&all_strs[j*STRLEN]);
        }
  } 
  return MPI_Finalize();
}
    
