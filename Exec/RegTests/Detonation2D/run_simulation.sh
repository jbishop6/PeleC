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

# === Paths ===
LHS_DIR="/mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs/lhs_samples"
LHS_RESULTS="/mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs/lhs_results.csv"

# === Wait briefly to ensure filesystem visibility ===
echo "[INFO] Waiting for filesystem to stabilize..."
sync
sleep 5

# === Debug: List LHS sample dir contents ===
echo "[DEBUG] Contents of $LHS_DIR:"
ls -lh "$LHS_DIR"

# === Step 1: Count number of usable LHS results ===
if [[ -f "$LHS_RESULTS" ]]; then
    result_count=$(awk 'END {print NR-1}' "$LHS_RESULTS")
else
    result_count=0
fi

echo "[INFO] Found $result_count entries in lhs_results.csv"

# === Step 2: Generate LHS samples if needed ===
if [[ $result_count -lt 5 ]]; then
    echo "[INFO] Not enough LHS results ($result_count/5) — generating LHS samples..."
    
    # Generate new LHS inputs
    python run_lhs_sampling.py

    # Submit job array for samples
    LHS_JOB_ID=$(sbatch --parsable run_sample.sh)
    echo "[INFO] LHS job array submitted as Job ID: $LHS_JOB_ID"

    # Wait for job array to complete
    echo "[INFO] Waiting for LHS simulations to complete..."
    while squeue -j "$LHS_JOB_ID" > /dev/null 2>&1 && squeue -j "$LHS_JOB_ID" | grep -q "$LHS_JOB_ID"; do
        echo "[WAIT] LHS samples still running..."
        sleep 60
    done

    echo "[INFO] All LHS simulations completed."
else
    echo "[INFO] Found enough LHS results. Skipping LHS generation."
fi

# === Step 3: Collect results from output folders ===
echo "[INFO] Collecting any new LHS results..."
python collect_lhs_results.py

# === Step 4: Run Bayesian optimization ===
echo "[INFO] Starting Bayesian Optimization..."
python optimize_geometry.py

echo "================================================="
echo " Job $SLURM_JOB_ID finished at $(date)"
echo "================================================="
