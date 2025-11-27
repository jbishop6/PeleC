# run_lhs_sample.py
import sys
import csv
import os
import traceback
from optimize_geometry import run_pelec_and_extract_thrust

SCRATCH_BASE = "/mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs"
INP_FILE = "inputs.detonation.threebranch.inp"
SIM_EXECUTABLE = "/home/jbishop6/PeleC/Exec/RegTests/Detonation2D/PeleC2d.gnu.ex"

# === Parse geometry args from CLI ===
try:
    geo_Z, geo_X, geo_H, geo_W = map(float, sys.argv[1:5])
except ValueError:
    print("[❌ ERROR] Invalid arguments passed to run_lhs_sample.py", file=sys.stderr)
    sys.exit(1)

try:
    print(f"[INFO] Starting LHS sample: Z={geo_Z}, X={geo_X}, H={geo_H}, W={geo_W}")
    
    # === Run simulation ===
    stats = run_pelec_and_extract_thrust(
        geo_Z, geo_X, geo_H, geo_W,
        INP_FILE, SIM_EXECUTABLE
    )

    # === Save results ===
    result_filename = f"result_Z{geo_Z}_X{geo_X}_H{geo_H}_W{geo_W}.csv".replace('.', 'p')
    result_path = os.path.join(SCRATCH_BASE, "lhs_samples", result_filename)

    with open(result_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["geo.Z", "geo.X", "geo.H", "geo.W", "thrust_avg", "thrust_std", "thrust_max", "thrust_min"])
        writer.writerow([
            geo_Z, geo_X, geo_H, geo_W,
            stats['thrust_avg'], stats['thrust_std'],
            stats['thrust_max'], stats['thrust_min']
        ])

    print(f"[✅ SUCCESS] Wrote results to {result_path}")

except Exception as e:
    print("[❌ ERROR] Simulation or logging failed:", file=sys.stderr)
    traceback.print_exc()
    sys.exit(1)
