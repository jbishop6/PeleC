#!/bin/bash
#SBATCH --job-name=2B_L0.080_W0.040_symmetric
#SBATCH -p compute-long
#SBATCH --output=2B_L0.080_W0.040_symmetric.out
#SBATCH --error=2B_L0.080_W0.040_symmetric.err
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

echo "Launching ./PeleC2d.gnu.MPI.ex input_detonation_two_branch.inp"
# srun ./PeleC2d.gnu.MPI.ex input_detonation_two_branch.inp

mpirun -np 16 ./PeleC2d.gnu.MPI.ex input_detonation_two_branch.inp

echo "Job $SLURM_JOB_ID finished at $(date)"
