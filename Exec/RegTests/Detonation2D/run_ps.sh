#!/bin/bash
#SBATCH --job-name=thirdbranch_sweep
#SBATCH --output=slurm_sweep_out_%j.log
#SBATCH --error=slurm_sweep_err_%j.log
#SBATCH --partition=bigmem-long
#SBATCH --nodes=1
#SBATCH --ntasks=1

echo "================================================="
echo " Parameter sweep job $SLURM_JOB_ID started on $(hostname) at $(date)"
echo "================================================="

# === Setup Conda environment ===
source ~/miniconda3/etc/profile.d/conda.sh
conda activate pelecopt

# Move to directory where you ran sbatch from
cd "$SLURM_SUBMIT_DIR"

echo "[INFO] Starting third-branch parameter sweep..."
python thirdbranch_parameter_sweep.py

echo "================================================="
echo " Parameter sweep job $SLURM_JOB_ID finished at $(date)"
echo "================================================="
