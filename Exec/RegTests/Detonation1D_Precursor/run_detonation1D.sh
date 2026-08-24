#!/bin/bash
#SBATCH --job-name=det1d
#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --output=slurm-%j.out
#SBATCH --error=slurm-%j.err

cd "$SLURM_SUBMIT_DIR"

# ------------------------------------------------------------
# Create timestamped results directory
# ------------------------------------------------------------
timestamp=$(date +"%Y-%m-%d_%H%M%S")
results_dir="results_${timestamp}"

mkdir -p "$results_dir"

echo "========================================"
echo "Detonation precursor run"
echo "Job ID: $SLURM_JOB_ID"
echo "Started: $(date)"
echo "Results directory: $results_dir"
echo "========================================"

# ------------------------------------------------------------
# Run PeleC
# ------------------------------------------------------------
echo "Hostname: $(hostname)"
echo "SLURM_JOB_ID=$SLURM_JOB_ID"
echo "SLURM_NODELIST=$SLURM_NODELIST"
echo "SLURM_NTASKS=$SLURM_NTASKS"
echo "SLURM_TASKS_PER_NODE=$SLURM_TASKS_PER_NODE"

echo "mpirun: $(which mpirun)"
mpirun --version

mpirun \
    --mca plm slurm \
    --mca ras slurm \
    -np "$SLURM_NTASKS" \
    ./PeleC2d.gnu.MPI.ex input.detonation1D.inp

run_status=$?

echo "========================================"
echo "PeleC exit status: $run_status"
echo "Finished: $(date)"
echo "========================================"

# ------------------------------------------------------------
# Move simulation results
# ------------------------------------------------------------

# Plotfiles
mv plt* "$results_dir"/ 2>/dev/null

# Checkpoint files, if present
mv chk* "$results_dir"/ 2>/dev/null

# PeleC data log, if present
mv datlog "$results_dir"/ 2>/dev/null

# ------------------------------------------------------------
# Save exact configuration used for this run
# ------------------------------------------------------------
cp input.detonation1D.inp "$results_dir"/
cp prob.cpp "$results_dir"/
cp prob.H "$results_dir"/
cp prob_parm.H "$results_dir"/

# ------------------------------------------------------------
# Create run information file
# ------------------------------------------------------------
{
    echo "Detonation 1-D Precursor Run"
    echo "============================"
    echo "Job ID: $SLURM_JOB_ID"
    echo "Timestamp: $timestamp"
    echo "MPI tasks: $SLURM_NTASKS"
    echo "Exit status: $run_status"
} > "$results_dir/run_info.txt"

# ------------------------------------------------------------
# Move Slurm output/error logs into results directory
# ------------------------------------------------------------
mv "slurm-${SLURM_JOB_ID}.out" "$results_dir"/ 2>/dev/null
mv "slurm-${SLURM_JOB_ID}.err" "$results_dir"/ 2>/dev/null

exit $run_status
