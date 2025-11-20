import numpy as np
import os
from optimize_geometry import run_pelec_and_extract_thrust, is_valid_input, generate_valid_lhs_samples
import csv

# Setup
INP_FILE = "inputs.detonation.threebranch.inp"
SIM_EXECUTABLE = "./PeleC2d.gnu.ex"
RESULTS_FILE = "lhs_results.csv"
os.makedirs("lhs_samples", exist_ok=True)

# Write header if not already there
if not os.path.exists(RESULTS_FILE):
    with open(RESULTS_FILE, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["geo.Z", "geo.X", "geo.H", "geo.W", "thrust_avg", "thrust_std", "thrust_max", "thrust_min"])

# Generate LHS samples
bounds = np.array([
    [0.05, 0.5],   # Z
    [0.15, 0.6],   # X
    [0.1, 0.6],    # H
    [0.05, 0.25]   # W
])
n_samples = 5

X_samples = generate_valid_lhs_samples(bounds, n_samples=n_samples)

# Run simulations sequentially and log to file
for i, x in enumerate(X_samples):
    geo_Z, geo_X, geo_H, geo_W = x
    print(f"[INFO] Running LHS sample {i + 1}/{n_samples}...")

    try:
        thrust_stats = run_pelec_and_extract_thrust(
            geo_Z, geo_X, geo_H, geo_W,
            INP_FILE, SIM_EXECUTABLE,
            iteration=None
        )

        with open(RESULTS_FILE, "a", newline="") as f:
            writer = csv.writer(f)
            writer.writerow([
                geo_Z, geo_X, geo_H, geo_W,
                thrust_stats['thrust_avg'],
                thrust_stats['thrust_std'],
                thrust_stats['thrust_max'],
                thrust_stats['thrust_min']
            ])

    except Exception as e:
        print(f"[WARN] LHS sample {i+1} failed: {e}")
