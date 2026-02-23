#!/bin/bash
#SBATCH --job-name=TBvsTB3_setup
#SBATCH --output=TBvsTB3_setup_%j.log
#SBATCH --error=TBvsTB3_setup_%j.err
#SBATCH --partition=compute-long
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1

echo "================================================="
echo " Sweep setup job $SLURM_JOB_ID started on $(hostname) at $(date)"
echo "================================================="

# === Environment Setup ===
source ~/miniconda3/etc/profile.d/conda.sh
conda activate pelecopt

# Go to Detonation2D folder
cd ~/PeleC/Exec/RegTests/Detonation2D

# === Run Python script to generate all run directories + run_job.sh's ===
echo "[INFO] Running make_TB_vs_TB3_runs.py"
python make_TB_vs_TB3_runs.py

# === Submit all individual geometry jobs ===
echo "[INFO] Submitting all geometric cases via submit_all.sh"
./TB_vs_TB3/submit_all.sh

echo "================================================="
echo " Sweep setup job $SLURM_JOB_ID finished at $(date)"
echo "================================================="
