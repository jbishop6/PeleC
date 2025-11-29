#!/bin/bash
#SBATCH --job-name=extract_thrust
#SBATCH --output=process_output_%j.log
#SBATCH --error=process_error_%j.log
#SBATCH --partition=compute-long
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=5

echo "================================================="
echo " Extract thrust from existing output folders"
echo "================================================="

source ~/miniconda3/etc/profile.d/conda.sh
conda activate pelecopt

cd "$SLURM_SUBMIT_DIR"

# Call a script that walks through all output dirs and extracts thrust
python process_existing.py

echo "Extraction job complete."
