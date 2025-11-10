#!/bin/bash
#SBATCH --job-name=geom_opt         # Name of the job
#SBATCH --output=output.log         # Where to write the output
#SBATCH --error=error.log           # Where to write errors
#SBATCH --time=01:00:00             # Max run time (hh:mm:ss)
#SBATCH --partition=standard        # Partition/queue to submit to (may vary)
#SBATCH --ntasks=1                  # Number of tasks (usually 1 for a Python script)
#SBATCH --cpus-per-task=4           # CPUs per task
#SBATCH --mem=4G                    # Memory per node

echo "Job started on $(hostname) at $(date)"

# Load required modules (if needed)
module load python/3.10  # You may need to adjust this

# Run your Python script
python optimize_geometry.py

echo "Job finished at $(date)"
