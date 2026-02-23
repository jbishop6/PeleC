# run_lhs_sampling.py (new version)
import numpy as np
import os
from scipy.stats.qmc import LatinHypercube
from optimize_geometry import is_valid_input

# === Parameters ===
SCRATCH_BASE = "~/PeleC/Exec/RegTests/Detonation2D/outputs_new"
os.makedirs(os.path.join(SCRATCH_BASE, "lhs_samples"), exist_ok=True)

bounds = np.array([
    [0.08, 0.15],   # Z
    [0.30, 0.50],   # X
    [0.10, 0.20],   # H
    [0.08, 0.15]    # W
])

n_samples_desired = 5
max_attempts = 100

# === Generate valid LHS samples ===
sampler = LatinHypercube(d=bounds.shape[0])
samples = []
attempts = 0

while len(samples) < n_samples_desired and attempts < max_attempts:
    x = bounds[:, 0] + sampler.random()[0] * (bounds[:, 1] - bounds[:, 0])
    if is_valid_input(x):
        samples.append(x)
    attempts += 1

if len(samples) < n_samples_desired:
    raise RuntimeError("❌ Not enough valid LHS samples.")

# === Save to CSV with header ===
lhs_path = os.path.join(SCRATCH_BASE, "lhs_samples", "lhs_input.csv")
with open(lhs_path, "w") as f:
    f.write("geo.Z,geo.X,geo.H,geo.W\n")
    for sample in samples:
        f.write(",".join(map(str, sample)) + "\n")

print(f"[INFO] Wrote {len(samples)} samples to {lhs_path}")

