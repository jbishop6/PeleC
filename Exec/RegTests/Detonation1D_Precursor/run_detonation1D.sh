#!/bin/bash
#SBATCH --job-name=baurle_val
#SBATCH --partition=compute-long
#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --output=slurm-%j.out
#SBATCH --error=slurm-%j.err

cd "$SLURM_SUBMIT_DIR"

# ------------------------------------------------------------
# Create timestamped validation results directory
# ------------------------------------------------------------
timestamp=$(date +"%Y-%m-%d_%H%M%S")
results_dir="validation_Brown1999_Baurle_1291K_3.31atm_30.4us_${timestamp}"

mkdir -p "$results_dir"

echo "========================================"
echo "Shepherd induction-time validation"
echo "Mechanism: Baurle-Eklund"
echo "Brown 1999 experimental case"
echo "Job ID: $SLURM_JOB_ID"
echo "Started: $(date)"
echo "Results directory: $results_dir"
echo "========================================"

# ------------------------------------------------------------
# Save exact configuration BEFORE running
# ------------------------------------------------------------
cp input.detonation1D.inp "$results_dir"/
cp prob.cpp "$results_dir"/
cp prob.H "$results_dir"/
cp prob_parm.H "$results_dir"/
cp GNUmakefile "$results_dir"/
cp PeleC2d.gnu.MPI.ex "$results_dir"/

# ------------------------------------------------------------
# Create run information file
# ------------------------------------------------------------
{
    echo "Shepherd Induction-Time Validation"
    echo "=================================="
    echo "Mechanism: Baurle-Eklund"
    echo "Experimental dataset: Brown 1999"
    echo "Mixture: C2H4 + 3 O2 + 12 N2"
    echo "Temperature: 1291 K"
    echo "Pressure: 3.31 atm"
    echo "Experimental induction time: 30.4 us"
    echo ""
    echo "Job ID: $SLURM_JOB_ID"
    echo "Timestamp: $timestamp"
    echo "MPI tasks requested: $SLURM_NTASKS"
} > "$results_dir/run_info.txt"

# ------------------------------------------------------------
# Run INSIDE the isolated validation directory
# ------------------------------------------------------------
cd "$results_dir"

echo "========================================"
echo "Running from: $(pwd)"
echo "Hostname: $(hostname)"
echo "SLURM_JOB_ID=$SLURM_JOB_ID"
echo "SLURM_NODELIST=$SLURM_NODELIST"
echo "SLURM_NTASKS=$SLURM_NTASKS"
echo "SLURM_TASKS_PER_NODE=$SLURM_TASKS_PER_NODE"
echo "========================================"

echo "mpirun: $(which mpirun)"
mpirun --version

mpirun --oversubscribe -np 2 ./PeleC2d.gnu.MPI.ex input.detonation1D.inp

run_status=$?

echo "========================================"
echo "PeleC exit status: $run_status"
echo "Finished: $(date)"
echo "========================================"

# ------------------------------------------------------------
# Append completion information
# ------------------------------------------------------------
{
    echo ""
    echo "Exit status: $run_status"
    echo "Finished: $(date)"
} >> run_info.txt

# ------------------------------------------------------------
# Move Slurm logs into this validation directory
# Slurm created them in the original submission directory.
# ------------------------------------------------------------
mv "${SLURM_SUBMIT_DIR}/slurm-${SLURM_JOB_ID}.out" . 2>/dev/null
mv "${SLURM_SUBMIT_DIR}/slurm-${SLURM_JOB_ID}.err" . 2>/dev/null

exit $run_status
