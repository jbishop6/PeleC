#!/bin/bash
#SBATCH --job-name=tube_branch
#SBATCH --output=slurm_output_%j.log
#SBATCH --error=slurm_error_%j.log
#SBATCH --partition=compute-long
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=5


echo "================================================="
echo " Job $SLURM_JOB_ID started on $(hostname) at $(date)"
echo "================================================="

# === Environment Setup ===
source ~/miniconda3/etc/profile.d/conda.sh
conda activate pelecopt
cd ~/PeleC/Exec/RegTests/Detonation_split

# === Create output directory ===
OUTDIR="TubeBranch_outputs"
mkdir -p "$OUTDIR"

# === Run PeleC with two-branch input file ===
./PeleC2d.gnu.MPI.ex input_detonation_tubebranch.inp

# === Move plotfiles to output folder ===
mv plt* "$OUTDIR"

# === Run thrust analysis on results ===
# python -c "
# from optimize_geometry import analyze_thrust_timeseries, get_all_plotfiles
# import os
# plot_dir = os.path.abspath('$OUTDIR')
# plotfiles = get_all_plotfiles(plot_dir)
# analyze_thrust_timeseries(plotfiles, plot_dir)
#"

echo "================================================="
echo " Job $SLURM_JOB_ID finished at $(date)"
echo "================================================="
