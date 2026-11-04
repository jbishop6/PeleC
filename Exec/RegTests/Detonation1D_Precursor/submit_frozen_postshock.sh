
#!/bin/bash
#SBATCH --job-name=frozen_shock
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --output=frozen_shock_%j.out
#SBATCH --error=frozen_shock_%j.err

# ------------------------------------------------------------
# GO TO DIRECTORY WHERE JOB WAS SUBMITTED
# ------------------------------------------------------------

cd "$SLURM_SUBMIT_DIR" || exit 1

echo "========================================"
echo "Cantera Frozen Post-Shock Analysis"
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
echo "Checking Cantera and SDToolbox..."

python -c "
import cantera as ct
from sdtoolbox.postshock import PostShock_fr

print('Cantera version:', ct.__version__)
print('SDToolbox imported successfully')
"

if [ $? -ne 0 ]; then
    echo "ERROR: Cantera or SDToolbox is unavailable."
    exit 1
fi

# ------------------------------------------------------------
# CHECK PYTHON SCRIPT
# ------------------------------------------------------------

if [ ! -f "calculate_frozen_postshock.py" ]; then
    echo "ERROR: calculate_frozen_postshock.py not found."
    exit 1
fi

# ------------------------------------------------------------
# RUN FROZEN POST-SHOCK CALCULATION
# ------------------------------------------------------------

echo ""
echo "Starting frozen post-shock calculation..."
echo ""

python calculate_frozen_postshock.py

run_status=$?

# ------------------------------------------------------------
# FINISH
# ------------------------------------------------------------

echo ""
echo "========================================"
echo "Frozen post-shock calculation finished"
echo "Exit status: $run_status"
echo "Finished: $(date)"
echo "========================================"

conda deactivate

exit $run_status
