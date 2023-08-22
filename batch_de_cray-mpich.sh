#!/bin/bash -l
#PBS -A SCSG0001
#PBS -N affinity
#PBS -j oe
#PBS -q main
#PBS -l walltime=00:20:00
#PBS -l select=2:ncpus=128:mpiprocs=16:ompthreads=8
#PBS -l place=scatter

module --force purge
module load ncarenv/23.06 craype/2.7.20 intel/2023.0.0 ncarcompilers/1.0.0 cray-mpich/8.1.25
export MPICH_MEMORY_REPORT=1

#export PATH=$PATH:/glade/work/rory/mpibind/
#mpibind balanced ./donde >& donde.out
export OMP_PLACES=threads
export OMP_PROC_BIND=close
mpiexec -n 32 -ppn 16 -d 8 --cpu-bind depth ./donde > donde.out
