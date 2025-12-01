#!/bin/bash
#SBATCH --job-name=opt_geo
#SBATCH --output=slurm_output_%j.log
#SBATCH --error=slurm_error_%j.log
#SBATCH --partition=bigmem-long
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=5
#SBATCH --mem=64G


echo "================================================="
echo " Job $SLURM_JOB_ID started on $(hostname) at $(date)"
echo "================================================="

# === Setup ===
source ~/miniconda3/etc/profile.d/conda.sh
conda activate pelecopt
cd "$SLURM_SUBMIT_DIR"

echo "[INFO] Starting Bayesian Optimization..."
python optimize_geometry.py

echo "================================================="
echo " Job $SLURM_JOB_ID finished at $(date)"
echo "================================================="
