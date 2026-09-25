#!/bin/bash
#SBATCH --job-name=gri_brown1291
#SBATCH --partition=compute-long
#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --output=slurm-%j.out
#SBATCH --error=slurm-%j.err

cd "$SLURM_SUBMIT_DIR"

# ------------------------------------------------------------
# Create timestamped results directory
# ------------------------------------------------------------
timestamp=$(date +"%Y-%m-%d_%H%M%S")

case_name="Brown1999_GRI_1291K_3.31atm_30.4us"
results_dir="${SLURM_SUBMIT_DIR}/validation_${case_name}_${timestamp}"

mkdir -p "$results_dir"

echo "========================================"
echo "Detonation precursor run"
echo "Job ID: $SLURM_JOB_ID"
echo "Started: $(date)"
echo "Results directory: $results_dir"
echo "========================================"

# ------------------------------------------------------------
# Save exact configuration used for this run
# ------------------------------------------------------------
cp input.detonation1D.inp "$results_dir"/
cp prob.cpp "$results_dir"/
cp prob.H "$results_dir"/
cp prob_parm.H "$results_dir"/
cp GNUmakefile "$results_dir"/
cp ./PeleC2d.gnu.MPI.ex "$results_dir"/

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

cd "$results_dir"

mpirun --oversubscribe -np 2 ./PeleC2d.gnu.MPI.ex input.detonation1D.inp

run_status=$?

echo "========================================"
echo "PeleC exit status: $run_status"
echo "Finished: $(date)"
echo "========================================"

# ------------------------------------------------------------
# Create run information file
# ------------------------------------------------------------
{
    echo "Shepherd Induction-Time Validation"
    echo "=================================="
    echo "Reference: Brown 1999 / Shepherd ethylene database"
    echo "Mechanism: GRI-Mech 3.0"
    echo "Mixture: C2H4 + 3 O2 + 12 N2"
    echo "Temperature: 1291 K"
    echo "Pressure: 3.31 atm"
    echo "Experimental induction time: 30.4 us"
    echo ""
    echo "Job ID: $SLURM_JOB_ID"
    echo "Timestamp: $timestamp"
    echo "MPI tasks launched: 2"
    echo "Exit status: $run_status"
} > "$results_dir/run_info.txt"

# ------------------------------------------------------------
# Move Slurm output/error logs into results directory
# ------------------------------------------------------------
mv "${SLURM_SUBMIT_DIR}/slurm-${SLURM_JOB_ID}.out" "$results_dir"/ 2>/dev/null
mv "${SLURM_SUBMIT_DIR}/slurm-${SLURM_JOB_ID}.err" "$results_dir"/ 2>/dev/null

exit $run_status
