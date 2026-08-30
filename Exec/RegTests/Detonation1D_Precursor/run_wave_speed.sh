#!/bin/bash
#SBATCH --job-name=wave_speed
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --output=wave_speed_%j.out
#SBATCH --error=wave_speed_%j.err

# ------------------------------------------------------------
# Go to directory where job was submitted
# ------------------------------------------------------------

cd "$SLURM_SUBMIT_DIR"

echo "========================================"
echo "PeleC Wave Speed Analysis"
echo "Job ID: $SLURM_JOB_ID"
echo "Started: $(date)"
echo "Working directory: $(pwd)"
echo "========================================"

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
echo "Checking yt installation..."

python -c "import yt; print('yt version:', yt.__version__)"

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: yt is not installed in the detonation environment."
    echo "Run:"
    echo "    conda activate detonation"
    echo "    python -m pip install yt"
    exit 1
fi

# ------------------------------------------------------------
# RUN WAVE SPEED ANALYSIS
# ------------------------------------------------------------

echo ""
echo "Starting PeleC wave-speed analysis..."
echo ""

python analyze_wave_speed.py

run_status=$?

# ------------------------------------------------------------
# FINISH
# ------------------------------------------------------------

echo ""
echo "========================================"
echo "Wave speed analysis finished"
echo "Exit status: $run_status"
echo "Finished: $(date)"
echo "========================================"

conda deactivate

exit $run_status
