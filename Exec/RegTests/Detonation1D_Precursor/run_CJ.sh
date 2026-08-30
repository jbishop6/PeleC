#!/bin/bash
#SBATCH --job-name=CJ_speed
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --output=CJ_speed_%j.out
#SBATCH --error=CJ_speed_%j.err

# ------------------------------------------------------------
# Go to directory where job was submitted
# ------------------------------------------------------------

cd "$SLURM_SUBMIT_DIR"

echo "========================================"
echo "CJ Detonation Calculation"
echo "Job ID: $SLURM_JOB_ID"
echo "Started: $(date)"
echo "Working directory: $(pwd)"
echo "========================================"

# ------------------------------------------------------------
# INITIALIZE CONDA
# ------------------------------------------------------------

# Load conda into this non-interactive shell
source /home/jbishop6/miniconda3/etc/profile.d/conda.sh
conda activate detonation

export PYTHONPATH="/home/jbishop6/sdtoolbox/Python3:$PYTHONPATH"

echo ""
echo "Python environment:"
echo "CONDA_DEFAULT_ENV = $CONDA_DEFAULT_ENV"
echo "Python = $(which python)"
python --version

# ------------------------------------------------------------
# RUN CJ CALCULATION
# ------------------------------------------------------------

echo ""
echo "Starting CJ calculation..."
echo ""

python CJ_speed.py

run_status=$?

# ------------------------------------------------------------
# FINISH
# ------------------------------------------------------------

echo ""
echo "========================================"
echo "CJ calculation finished"
echo "Exit status: $run_status"
echo "Finished: $(date)"
echo "========================================"

conda deactivate

exit $run_status
