#!/bin/bash
#SBATCH --job-name=lhs_sample
#SBATCH --output=logs/lhs_%A_%a.out
#SBATCH --error=logs/lhs_%A_%a.err
#SBATCH --array=0-4
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --partition=compute

# === Environment Setup ===
source ~/miniconda3/etc/profile.d/conda.sh
conda activate pelecopt
cd "$SLURM_SUBMIT_DIR"

# === File and Line Parsing ===
LHS_INPUT="/mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs/lhs_samples/lhs_input.csv"
# Read the sample line based on array task ID
line=$(tail -n +2 /mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs/lhs_samples/lhs_input.csv | sed -n "$((SLURM_ARRAY_TASK_ID + 1))p")

# Exit if line is empty
if [ -z "$line" ]; then
    echo "[ERROR] No valid line found for task ID $SLURM_ARRAY_TASK_ID"
    exit 1
fi

Z=$(echo $line | cut -d',' -f1)
X=$(echo $line | cut -d',' -f2)
H=$(echo $line | cut -d',' -f3)
W=$(echo $line | cut -d',' -f4)

# === Run the Simulation ===
python run_lhs_sample.py "$Z" "$X" "$H" "$W"
