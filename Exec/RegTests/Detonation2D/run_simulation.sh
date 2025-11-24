#!/bin/bash
#SBATCH --job-name=opt_geo
#SBATCH --output=slurm_output_%j.log
#SBATCH --error=slurm_error_%j.log
#SBATCH --partition=compute-long
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=5
#SBATCH --time=2-00:00:00

echo "================================================="
echo "🔧 Job $SLURM_JOB_ID started on $(hostname) at $(date)"
echo "================================================="

# === Setup ===
source ~/miniconda3/etc/profile.d/conda.sh
conda activate pelecopt
cd "$SLURM_SUBMIT_DIR"

# === Step 1: Generate LHS samples ===
echo "[INFO] Generating LHS samples..."
python run_lhs_sampling.py

# === Step 2: Launch LHS simulations in parallel ===
LHS_INPUT="/mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs/lhs_samples/lhs_input.csv"
echo "[INFO] Running simulations in parallel..."

i=0
while IFS=',' read -r Z X H W; do
    echo "[LAUNCH] Sample $((i+1)): Z=$Z X=$X H=$H W=$W"
    python run_lhs_sample.py "$Z" "$X" "$H" "$W" &
    ((i++))
done < "$LHS_INPUT"

# === Step 3: Wait for all to finish ===
wait
echo "[INFO] All LHS simulations completed."

# === Step 4: Merge LHS results ===
echo "[INFO] Merging LHS results..."
cd /mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs/lhs_samples

head -n 1 result_*.csv | head -n 1 > ../lhs_results.csv
tail -n +2 -q result_*.csv >> ../lhs_results.csv

# === Step 5: Run Bayesian optimization ===
echo "[INFO] Starting Bayesian Optimization..."
cd "$SLURM_SUBMIT_DIR"
python optimize_geometry.py

echo "================================================="
echo "✅ Job $SLURM_JOB_ID finished at $(date)"
echo "================================================="
