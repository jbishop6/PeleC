#!/bin/bash
#SBATCH --job-name=3B_L0.080_W0.140_symmetric
#SBATCH -p compute-long
#SBATCH --output=3B_L0.080_W0.140_symmetric.out
#SBATCH --error=3B_L0.080_W0.140_symmetric.err
#SBATCH --ntasks=16
#SBATCH --cpus-per-task=1

set -x  # echo commands as they run

# Run from the directory containing this script
cd "$SLURM_SUBMIT_DIR"

# OpenMP threads: match cpus-per-task
export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK}

echo "Job $SLURM_JOB_ID running on $(hostname) in $(pwd)"
echo "MPI ranks: $SLURM_NTASKS, OMP threads: $OMP_NUM_THREADS"
date

echo "Launching ./PeleC2d.gnu.MPI.ex inputs.detonation.threebranch.inp"
# srun ./PeleC2d.gnu.MPI.ex inputs.detonation.threebranch.inp

mpirun -np 16 ./PeleC2d.gnu.MPI.ex inputs.detonation.threebranch.inp

echo "Job $SLURM_JOB_ID finished at $(date)"
