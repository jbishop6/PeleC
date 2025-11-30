#!/bin/bash
#SBATCH --job-name=opt_geo
#SBATCH --output=slurm_output_%j.log
#SBATCH --error=slurm_error_%j.log
#SBATCH --partition=compute-long
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=5

echo "================================================="
echo " Job $SLURM_JOB_ID started on $(hostname) at $(date)"
echo "================================================="

# === Setup ===
source ~/miniconda3/etc/profile.d/conda.sh
conda activate pelecopt
cd "$SLURM_SUBMIT_DIR"

LHS_DIR="/mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs/lhs_samples"
LHS_RESULTS="/mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs/lhs_results.csv"

# === Step 1: Use Python script to decide if we need more LHS samples ===
python check_lhs_sample_count.py
NEED_SAMPLES=$?

if [[ $NEED_SAMPLES -eq 1 ]]; then
    echo "[INFO] Need to generate more LHS samples."
    python run_lhs_sampling.py

    echo "[INFO] Submitting LHS simulations via job array..."
    LHS_JOB_ID=$(sbatch --parsable run_sample.sh)
    echo "[INFO] LHS job array submitted as Job ID: $LHS_JOB_ID"

    echo "[INFO] Waiting for all LHS simulations to complete..."
    while squeue -j "$LHS_JOB_ID" > /dev/null 2>&1 && squeue -j "$LHS_JOB_ID" | grep -q "$LHS_JOB_ID"; do
        echo "[WAIT] LHS samples still running..."
        sleep 60
    done

    echo "[INFO] All LHS simulations completed."
else
    echo "[INFO] Enough valid LHS samples found. Skipping sampling step."
fi

# === Step 2: Collect any missing results ===
echo "[INFO] Collecting LHS results..."
python collect_lhs_results.py

# === Step 3: Start Bayesian Optimization ===
echo "[INFO] Starting Bayesian Optimization..."
python optimize_geometry.py

echo "================================================="
echo " Job $SLURM_JOB_ID finished at $(date)"
echo "================================================="
