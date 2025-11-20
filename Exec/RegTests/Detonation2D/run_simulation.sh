#!/bin/bash
#SBATCH --job-name=opt_geo
#SBATCH --output=slurm_output.log
#SBATCH --error=slurm_error.log
#SBATCH --partition=compute-long
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=5

echo "Job started on $(hostname) at $(date)"

# === Initialize Conda ===
source ~/miniconda3/etc/profile.d/conda.sh

# === Activate your environment ===
conda activate pelecopt

# === Change to working directory ===
cd "$SLURM_SUBMIT_DIR"

# === Step 1: Run LHS sampling ===
echo "[INFO] Running Latin Hypercube Sampling..."
python run_lhs_sampling.py

# === Step 2: Run optimization based on LHS results ===
echo "[INFO] Starting Bayesian Optimization..."
python optimize_geometry.py

echo "Job finished at $(date)"
