#!/bin/bash
#SBATCH --job-name=lhs_sample
#SBATCH --output=logs/sample_%j.out
#SBATCH --error=logs/sample_%j.err
#SBATCH --partition=compute-long
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --time=02:00:00

echo "Started job on $(hostname) at $(date)"

source ~/miniconda3/etc/profile.d/conda.sh
conda activate pelecopt
cd "$SLURM_SUBMIT_DIR"

SAMPLE_FILE=$1
python run_sample_from_file.py "$SAMPLE_FILE"

echo "Finished job at $(date)"
