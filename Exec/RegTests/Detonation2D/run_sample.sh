#!/bin/bash
#SBATCH --job-name=lhs_sample
#SBATCH --output=logs/sample_%j.out
#SBATCH --error=logs/sample_%j.err
#SBATCH --partition=compute-long
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --export=ALL
#SBATCH --time=06:00:00
#SBATCH --shell=/bin/bash

echo "Started job on $(hostname) at $(date)"

# === Load Conda ===
source ~/miniconda3/etc/profile.d/conda.sh
conda activate pelecopt

# Debug environment
echo "[DEBUG] Python executable: $(which python)"
echo "[DEBUG] Python version: $(python --version)"
echo "[DEBUG] Conda env: $CONDA_DEFAULT_ENV"

# === Go to working directory ===
cd "$SLURM_SUBMIT_DIR"

# === Run sample ===
SAMPLE_FILE=$1
python run_sample_from_file.py "$SAMPLE_FILE"

echo "Finished job at $(date)"
