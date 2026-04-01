#!/bin/bash

# 2B_L0.080_W0.040_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.080_W0.040_symmetric"; then
    echo "Skipping 2B_L0.080_W0.040_symmetric: a job named 2B_L0.080_W0.040_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.080_W0.040" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.080_W0.040_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.080_W0.040_symmetric"; then
    echo "Skipping 3B_L0.080_W0.040_symmetric: a job named 3B_L0.080_W0.040_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.080_W0.040" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.080_W0.080_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.080_W0.080_symmetric"; then
    echo "Skipping 2B_L0.080_W0.080_symmetric: a job named 2B_L0.080_W0.080_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.080_W0.080" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.080_W0.080_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.080_W0.080_symmetric"; then
    echo "Skipping 3B_L0.080_W0.080_symmetric: a job named 3B_L0.080_W0.080_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.080_W0.080" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.080_W0.100_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.080_W0.100_symmetric"; then
    echo "Skipping 2B_L0.080_W0.100_symmetric: a job named 2B_L0.080_W0.100_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.080_W0.100" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.080_W0.100_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.080_W0.100_symmetric"; then
    echo "Skipping 3B_L0.080_W0.100_symmetric: a job named 3B_L0.080_W0.100_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.080_W0.100" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.080_W0.120_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.080_W0.120_symmetric"; then
    echo "Skipping 2B_L0.080_W0.120_symmetric: a job named 2B_L0.080_W0.120_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.080_W0.120" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.080_W0.120_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.080_W0.120_symmetric"; then
    echo "Skipping 3B_L0.080_W0.120_symmetric: a job named 3B_L0.080_W0.120_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.080_W0.120" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.080_W0.140_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.080_W0.140_symmetric"; then
    echo "Skipping 2B_L0.080_W0.140_symmetric: a job named 2B_L0.080_W0.140_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.080_W0.140" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.080_W0.140_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.080_W0.140_symmetric"; then
    echo "Skipping 3B_L0.080_W0.140_symmetric: a job named 3B_L0.080_W0.140_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.080_W0.140" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.080_W0.160_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.080_W0.160_symmetric"; then
    echo "Skipping 2B_L0.080_W0.160_symmetric: a job named 2B_L0.080_W0.160_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.080_W0.160" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.080_W0.160_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.080_W0.160_symmetric"; then
    echo "Skipping 3B_L0.080_W0.160_symmetric: a job named 3B_L0.080_W0.160_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.080_W0.160" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.120_W0.040_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.120_W0.040_symmetric"; then
    echo "Skipping 2B_L0.120_W0.040_symmetric: a job named 2B_L0.120_W0.040_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.120_W0.040" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.120_W0.040_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.120_W0.040_symmetric"; then
    echo "Skipping 3B_L0.120_W0.040_symmetric: a job named 3B_L0.120_W0.040_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.120_W0.040" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.120_W0.080_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.120_W0.080_symmetric"; then
    echo "Skipping 2B_L0.120_W0.080_symmetric: a job named 2B_L0.120_W0.080_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.120_W0.080" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.120_W0.080_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.120_W0.080_symmetric"; then
    echo "Skipping 3B_L0.120_W0.080_symmetric: a job named 3B_L0.120_W0.080_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.120_W0.080" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.120_W0.100_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.120_W0.100_symmetric"; then
    echo "Skipping 2B_L0.120_W0.100_symmetric: a job named 2B_L0.120_W0.100_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.120_W0.100" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.120_W0.100_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.120_W0.100_symmetric"; then
    echo "Skipping 3B_L0.120_W0.100_symmetric: a job named 3B_L0.120_W0.100_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.120_W0.100" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.120_W0.120_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.120_W0.120_symmetric"; then
    echo "Skipping 2B_L0.120_W0.120_symmetric: a job named 2B_L0.120_W0.120_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.120_W0.120" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.120_W0.120_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.120_W0.120_symmetric"; then
    echo "Skipping 3B_L0.120_W0.120_symmetric: a job named 3B_L0.120_W0.120_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.120_W0.120" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.120_W0.140_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.120_W0.140_symmetric"; then
    echo "Skipping 2B_L0.120_W0.140_symmetric: a job named 2B_L0.120_W0.140_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.120_W0.140" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.120_W0.140_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.120_W0.140_symmetric"; then
    echo "Skipping 3B_L0.120_W0.140_symmetric: a job named 3B_L0.120_W0.140_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.120_W0.140" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.120_W0.160_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.120_W0.160_symmetric"; then
    echo "Skipping 2B_L0.120_W0.160_symmetric: a job named 2B_L0.120_W0.160_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.120_W0.160" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.120_W0.160_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.120_W0.160_symmetric"; then
    echo "Skipping 3B_L0.120_W0.160_symmetric: a job named 3B_L0.120_W0.160_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.120_W0.160" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.160_W0.040_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.160_W0.040_symmetric"; then
    echo "Skipping 2B_L0.160_W0.040_symmetric: a job named 2B_L0.160_W0.040_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.160_W0.040" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.160_W0.040_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.160_W0.040_symmetric"; then
    echo "Skipping 3B_L0.160_W0.040_symmetric: a job named 3B_L0.160_W0.040_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.160_W0.040" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.160_W0.080_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.160_W0.080_symmetric"; then
    echo "Skipping 2B_L0.160_W0.080_symmetric: a job named 2B_L0.160_W0.080_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.160_W0.080" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.160_W0.080_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.160_W0.080_symmetric"; then
    echo "Skipping 3B_L0.160_W0.080_symmetric: a job named 3B_L0.160_W0.080_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.160_W0.080" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.160_W0.100_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.160_W0.100_symmetric"; then
    echo "Skipping 2B_L0.160_W0.100_symmetric: a job named 2B_L0.160_W0.100_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.160_W0.100" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.160_W0.100_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.160_W0.100_symmetric"; then
    echo "Skipping 3B_L0.160_W0.100_symmetric: a job named 3B_L0.160_W0.100_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.160_W0.100" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.160_W0.120_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.160_W0.120_symmetric"; then
    echo "Skipping 2B_L0.160_W0.120_symmetric: a job named 2B_L0.160_W0.120_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.160_W0.120" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.160_W0.120_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.160_W0.120_symmetric"; then
    echo "Skipping 3B_L0.160_W0.120_symmetric: a job named 3B_L0.160_W0.120_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.160_W0.120" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.160_W0.140_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.160_W0.140_symmetric"; then
    echo "Skipping 2B_L0.160_W0.140_symmetric: a job named 2B_L0.160_W0.140_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.160_W0.140" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.160_W0.140_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.160_W0.140_symmetric"; then
    echo "Skipping 3B_L0.160_W0.140_symmetric: a job named 3B_L0.160_W0.140_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.160_W0.140" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 2B_L0.160_W0.160_symmetric
if squeue -u $USER -h -o "%j" | grep -q "2B_L0.160_W0.160_symmetric"; then
    echo "Skipping 2B_L0.160_W0.160_symmetric: a job named 2B_L0.160_W0.160_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_2B_L0.160_W0.160" && sbatch "run_job.sh" && cd - > /dev/null
fi

# 3B_L0.160_W0.160_symmetric
if squeue -u $USER -h -o "%j" | grep -q "3B_L0.160_W0.160_symmetric"; then
    echo "Skipping 3B_L0.160_W0.160_symmetric: a job named 3B_L0.160_W0.160_symmetric is already in the queue"
else
    cd "TB_vs_TB3/run_3B_L0.160_W0.160" && sbatch "run_job.sh" && cd - > /dev/null
fi

