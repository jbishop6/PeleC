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

# === Count existing result CSV files ===
existing_count=$(find "$LHS_DIR" -maxdepth 1 -name "result_*.csv" | wc -l)
echo "[INFO] Found $existing_count result_*.csv files"

# === Step 1: Generate LHS samples ===
if [[ $existing_count -lt 5 ]]; then
    echo "[INFO] Less than 5 results — generating LHS samples..."
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
    echo "[INFO] Found enough result folders. Skipping SLURM array job."
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
