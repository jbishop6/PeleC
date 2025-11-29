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

SCRATCH_DIR="/mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs"
LHS_DIR="$SCRATCH_DIR/lhs_samples"
LHS_INPUT="$LHS_DIR/lhs_input.csv"
LHS_RESULTS="$SCRATCH_DIR/lhs_results.csv"

# === Step 1: Generate LHS input samples if missing ===
if [ ! -f "$LHS_INPUT" ]; then
    echo "[INFO] lhs_input.csv not found — generating new LHS samples..."
    python run_lhs_sampling.py
else
    echo "[INFO] lhs_input.csv already exists — skipping generation."
fi

# === Step 2: Check how many results we already have ===
existing_csvs=($(ls "$LHS_DIR"/result_*.csv 2>/dev/null))
existing_count=${#existing_csvs[@]}
echo "[INFO] Found $existing_count existing result CSV files."

# === Step 3: Submit run_sample.sh only if needed ===
if [ "$existing_count" -lt 5 ]; then
    echo "[INFO] Submitting LHS job array to compute missing samples..."
    LHS_JOB_ID=$(sbatch --parsable run_sample.sh)
    echo "[INFO] LHS job array submitted as Job ID: $LHS_JOB_ID"

    echo "[INFO] Waiting for all LHS simulations to complete..."
    while squeue -j "$LHS_JOB_ID" > /dev/null 2>&1 && squeue -j "$LHS_JOB_ID" | grep -q "$LHS_JOB_ID"; do
        echo "[WAIT] LHS samples still running..."
        sleep 60
    done
    echo "[INFO] All LHS simulations completed."
else
    echo "[INFO] Sufficient LHS samples found — skipping SLURM job array."
fi

# === Step 4: Merge results into lhs_results.csv (always regenerate to be safe) ===
echo "[INFO] Merging all result_*.csv files into lhs_results.csv..."
cd "$LHS_DIR"
head -n 1 result_*.csv | head -n 1 > ../lhs_results.csv
tail -n +2 -q result_*.csv >> ../lhs_results.csv
cd "$SLURM_SUBMIT_DIR"

# === Step 5: Run Bayesian Optimization ===
echo "[INFO] Launching Bayesian Optimization (if ready)..."
python optimize_geometry.py

echo "================================================="
echo " Job $SLURM_JOB_ID finished at $(date)"
echo "================================================="
