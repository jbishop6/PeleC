#!/bin/bash
#SBATCH --job-name=opt_geo
#SBATCH --output=slurm_output.log
#SBATCH --error=slurm_error.log
#SBATCH --partition=compute-long
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4

echo "Job started on $(hostname) at $(date)"

# Load required modules (if needed)
module load python/3.10  # You may need to adjust this

# Run your Python script
python optimize_geometry.py

echo "Job finished at $(date)"
