import os
import numpy as np
import subprocess
import re
from scipy.stats.qmc import LatinHypercube

os.makedirs("lhs_samples", exist_ok=True)
os.makedirs("logs", exist_ok=True)

bounds = np.array([
    [0.05, 0.5],   # Z
    [0.15, 0.6],   # X
    [0.1, 0.6],    # H
    [0.05, 0.25]   # W
])

def is_valid_input(x):
    geo_X, geo_H, geo_Z, geo_W = x
    return (geo_X + geo_W <= 1.0) and (geo_H + geo_Z <= 2.0)

def generate_valid_lhs_samples(bounds, n_samples=5):
    sampler = LatinHypercube(d=bounds.shape[0])
    samples = []
    while len(samples) < n_samples:
        candidate = bounds[:, 0] + sampler.random(n=1)[0] * (bounds[:, 1] - bounds[:, 0])
        if is_valid_input(candidate):
            samples.append(candidate)
    return np.array(samples)

# Generate samples
X_init = generate_valid_lhs_samples(bounds, n_samples=5)

# Save and submit
job_ids = []
for i, x in enumerate(X_init):
    sample_file = f"lhs_samples/sample_{i}.txt"
    np.savetxt(sample_file, x)

    result = subprocess.run(["sbatch", "run_sample.sh", sample_file], capture_output=True, text=True)
    print(result.stdout.strip())

    match = re.search(r"Submitted batch job (\d+)", result.stdout)
    if match:
        job_ids.append(match.group(1))

print(f"Submitted {len(job_ids)} jobs. Check with: squeue -u $USER")
