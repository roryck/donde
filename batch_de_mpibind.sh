#!/bin/bash -l
#PBS -A SCSG0001
#PBS -N affinity
#PBS -j oe
#PBS -q main
#PBS -l walltime=00:20:00
#PBS -l select=2:ncpus=128:mpiprocs=20:ompthreads=4
#PBS -l place=scatter

module --force purge
#module load ncarenv/23.06 craype/2.7.20 intel/2023.0.0 ncarcompilers/1.0.0 cray-mpich/8.1.25
#module load ncarenv/23.06 craype/2.7.20 intel/2023.0.0 ncarcompilers/1.0.0 intel-mpi
module load ncarenv/23.06 craype/2.7.20 cce ncarcompilers/1.0.0 cray-mpich/8.1.25

export TMPDIR=/glade/derecho/scratch/rory/tmp
export PATH=$PATH:/glade/work/rory/mpibind/
export OMP_PROC_BIND=spread
export OMP_PLACES=cores
mpibind balanced ./donde >& donde.out
#mpiexec -n 32 -ppn 16 -d 8 --cpu-bind depth ./donde > donde.out
#mpiexec.hydra -n 120 -ppn 60 --bind-to none ./donde > donde.out
