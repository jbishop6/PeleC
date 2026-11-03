#!/bin/bash
#SBATCH --job-name=det_plots
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --output=det_plots_%j.out
#SBATCH --error=det_plots_%j.err

# ------------------------------------------------------------
# Go to directory where job was submitted
# ------------------------------------------------------------

cd "$SLURM_SUBMIT_DIR" || exit 1

echo "========================================"
echo "PeleC Detonation Profile Analysis"
echo "Job ID: $SLURM_JOB_ID"
echo "Started: $(date)"
echo "Working directory: $(pwd)"
echo "========================================"

# ------------------------------------------------------------
# SIMULATION RESULTS DIRECTORY
# ------------------------------------------------------------

RESULTS_DIR="/home/jbishop6/PeleC/Exec/RegTests/Detonation1D_Precursor/validation_Brown1999_Baurle_1291K_3.31atm_30.4us_2026-11-02_095632"

if [ ! -d "$RESULTS_DIR" ]; then
    echo "ERROR: Results directory does not exist:"
    echo "$RESULTS_DIR"
    exit 1
fi

echo "Analyzing results from: $RESULTS_DIR"

# ------------------------------------------------------------
# INITIALIZE CONDA
# ------------------------------------------------------------

source /home/jbishop6/miniconda3/etc/profile.d/conda.sh
conda activate detonation

echo ""
echo "Python environment:"
echo "CONDA_DEFAULT_ENV = $CONDA_DEFAULT_ENV"
echo "Python = $(which python)"
python --version

# ------------------------------------------------------------
# CHECK REQUIRED PYTHON PACKAGE
# ------------------------------------------------------------

echo ""
echo "Checking Python packages..."

python -c "
import yt
import numpy
import matplotlib
print('yt version:', yt.__version__)
print('NumPy version:', numpy.__version__)
print('Matplotlib version:', matplotlib.__version__)
"

if [ $? -ne 0 ]; then
    echo "ERROR: Required Python packages are missing."
    exit 1
fi

# ------------------------------------------------------------
# CHECK ANALYSIS SCRIPT
# ------------------------------------------------------------

if [ ! -f "plot_detonation_profiles.py" ]; then
    echo "ERROR: plot_detonation_profiles.py not found."
    exit 1
fi

# ------------------------------------------------------------
# RUN DETONATION PROFILE ANALYSIS
# ------------------------------------------------------------

echo ""
echo "Starting PeleC detonation profile analysis..."
echo ""

export MPLBACKEND=Agg

python plot_detonation_profiles.py

run_status=$?

# ------------------------------------------------------------
# FINISH
# ------------------------------------------------------------

echo ""
echo "========================================"
echo "Detonation profile analysis finished"
echo "Exit status: $run_status"
echo "Finished: $(date)"
echo "========================================"

conda deactivate

exit $run_status
