import numpy as np
import os
from optimize_geometry import run_pelec_and_extract_thrust, is_valid_input
import csv
from scipy.stats.qmc import LatinHypercube

# === Setup ===
INP_FILE = "inputs.detonation.threebranch.inp"
SIM_EXECUTABLE = "./PeleC2d.gnu.ex"
RESULTS_FILE = "lhs_results.csv"
os.makedirs("lhs_samples", exist_ok=True)

# === Bounds ===
bounds = np.array([
    [0.1, 0.3],    # Z (was 0.05 - 0.5)
    [0.2, 0.4],    # X (was 0.15 - 0.6)
    [0.2, 0.5],    # H (was 0.1 - 0.6)
    [0.08, 0.18]   # W (was 0.05 - 0.25)
])

n_samples_desired = 5
max_total_attempts = 30

# === Write header if not present ===
if not os.path.exists(RESULTS_FILE):
    with open(RESULTS_FILE, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["geo.Z", "geo.X", "geo.H", "geo.W", "thrust_avg", "thrust_std", "thrust_max", "thrust_min"])

# === Generate and Run Valid Samples ===
dim = bounds.shape[0]
sampler = LatinHypercube(d=dim)
n_collected = 0
n_attempted = 0

while n_collected < n_samples_desired and n_attempted < max_total_attempts:
    candidate = bounds[:, 0] + sampler.random()[0] * (bounds[:, 1] - bounds[:, 0])
    n_attempted += 1

    if not is_valid_input(candidate):
        print(f"[SKIP] Sample {n_attempted}: Invalid geometry {candidate}")
        continue

    geo_Z, geo_X, geo_H, geo_W = candidate
    print(f"[INFO] Running LHS sample {n_collected + 1}...")

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
        n_collected += 1

    except Exception as e:
        print(f"[WARN] Sample failed: {candidate} — {e}")
        continue

if n_collected == 0:
    raise RuntimeError("❌ All LHS samples failed. No data was collected.")
else:
    print(f"[INFO] Successfully collected {n_collected} LHS samples out of {n_attempted} attempts.")
