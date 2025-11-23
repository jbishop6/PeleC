#!/bin/bash
#SBATCH --job-name=opt_geo
#SBATCH --output=slurm_output_%j.log     # Job-specific output
#SBATCH --error=slurm_error_%j.log       # Job-specific error
#SBATCH --partition=compute-long
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=5

echo "================================================="
echo "🔧 Job $SLURM_JOB_ID started on $(hostname) at $(date)"
echo "================================================="

# === Initialize Conda ===
source ~/miniconda3/etc/profile.d/conda.sh
conda activate pelecopt

# === Change to working directory ===
cd "$SLURM_SUBMIT_DIR"

# === Step 1: Run LHS sampling ===
echo "[INFO] Running Latin Hypercube Sampling..."
python run_lhs_sampling.py

# === Step 2: Run optimization based on LHS results ===
echo "[INFO] Starting Bayesian Optimization..."
python optimize_geometry.py

echo "================================================="
echo "✅ Job $SLURM_JOB_ID finished at $(date)"
echo "================================================="
