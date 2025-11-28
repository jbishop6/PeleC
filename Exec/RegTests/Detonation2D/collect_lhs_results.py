# collect_lhs_results.py
import os
import re
import sys
from optimize_geometry import extract_thrust_from_existing_output

SCRATCH_BASE = "/mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs"

# Get all run directories
run_dirs = sorted([
    d for d in os.listdir(SCRATCH_BASE)
    if d.startswith("run_Z") and os.path.isdir(os.path.join(SCRATCH_BASE, d))
])

print(f"[INFO] Found {len(run_dirs)} LHS run directories")

for run_dir in run_dirs:
    full_path = os.path.join(SCRATCH_BASE, run_dir)

    # Parse geometry from folder name using regex
    match = re.search(
        r"Z(?P<Z>[\d_]+)_X(?P<X>[\d_]+)_H(?P<H>[\d_]+)_W(?P<W>[\d_]+)", run_dir.replace("p", "_")
    )
    if not match:
        print(f"[WARN] Skipping malformed directory: {run_dir}")
        continue

    try:
        geo_Z = float(match.group("Z").replace("_", "."))
        geo_X = float(match.group("X").replace("_", "."))
        geo_H = float(match.group("H").replace("_", "."))
        geo_W = float(match.group("W").replace("_", "."))

        print(f"[INFO] Processing {run_dir}...")

        extract_thrust_from_existing_output(
            Z=geo_Z, X=geo_X, H=geo_H, W=geo_W,
            run_directory=full_path
        )

    except Exception as e:
        print(f"[ERROR] Failed to extract from {run_dir}: {e}")
