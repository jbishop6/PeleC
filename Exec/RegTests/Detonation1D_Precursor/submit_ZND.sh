#!/bin/bash
#SBATCH --job-name=ZND_Baurle
#SBATCH --output=ZND_Baurle_%j.out
#SBATCH --error=ZND_Baurle_%j.err
#SBATCH --nodes=1
#SBATCH --ntasks=1

set -e

cd /home/jbishop6/PeleC/Exec/RegTests/Detonation1D_Precursor

# Activate existing Cantera / SDToolbox environment
source /home/jbishop6/miniconda3/etc/profile.d/conda.sh
conda activate detonation

echo "Starting ZND calculation"
date

python calculate_ZND.py

echo "ZND calculation finished"
date
