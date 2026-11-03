
#!/bin/bash
#SBATCH --job-name=ign_delay
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --output=ign_delay_%j.out
#SBATCH --error=ign_delay_%j.err

# ------------------------------------------------------------
# GO TO DIRECTORY WHERE JOB WAS SUBMITTED
# ------------------------------------------------------------

cd "$SLURM_SUBMIT_DIR" || exit 1

echo "========================================"
echo "Cantera Ignition Delay Analysis"
echo "Job ID: $SLURM_JOB_ID"
echo "Started: $(date)"
echo "Working directory: $(pwd)"
echo "========================================"

# ------------------------------------------------------------
# INITIALIZE CONDA
# ------------------------------------------------------------

source /home/jbishop6/miniconda3/etc/profile.d/conda.sh

conda activate detonation || exit 1

echo ""
echo "Python environment:"
echo "CONDA_DEFAULT_ENV = $CONDA_DEFAULT_ENV"
echo "Python = $(which python)"
python --version

# ------------------------------------------------------------
# CHECK REQUIRED PYTHON PACKAGES
# ------------------------------------------------------------

echo ""
echo "Checking Python packages..."

python -c "
import cantera as ct
import numpy as np
import matplotlib

print('Cantera version:', ct.__version__)
print('NumPy version:', np.__version__)
print('Matplotlib version:', matplotlib.__version__)
"

if [ $? -ne 0 ]; then
    echo "ERROR: Required Python packages are missing."
    exit 1
fi

# ------------------------------------------------------------
# CHECK IGNITION DELAY SCRIPT
# ------------------------------------------------------------

if [ ! -f "calculate_ignition_delay.py" ]; then
    echo "ERROR: calculate_ignition_delay.py not found."
    exit 1
fi

# ------------------------------------------------------------
# RUN CANTERA IGNITION DELAY ANALYSIS
# ------------------------------------------------------------

echo ""
echo "Starting Cantera ignition delay calculation..."
echo ""

export MPLBACKEND=Agg

python calculate_ignition_delay.py

run_status=$?

# ------------------------------------------------------------
# FINISH
# ------------------------------------------------------------

echo ""
echo "========================================"
echo "Ignition delay analysis finished"
echo "Exit status: $run_status"
echo "Finished: $(date)"
echo "========================================"

conda deactivate

exit $run_status
