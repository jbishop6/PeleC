# run_lhs_sample.py
import sys
import csv
import os
from optimize_geometry import run_pelec_and_extract_thrust

SCRATCH_BASE = "/mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs"
INP_FILE = "inputs.detonation.threebranch.inp"
SIM_EXECUTABLE = "/home/jbishop6/PeleC/Exec/RegTests/Detonation2D/PeleC2d.gnu.ex"

# Parse geometry args from SLURM array call
geo_Z, geo_X, geo_H, geo_W = map(float, sys.argv[1:5])

# Run simulation
try:
    stats = run_pelec_and_extract_thrust(
        geo_Z, geo_X, geo_H, geo_W,
        INP_FILE, SIM_EXECUTABLE
    )

    result_file = os.path.join(
        SCRATCH_BASE, "lhs_samples",
        f"result_Z{geo_Z}_X{geo_X}_H{geo_H}_W{geo_W}.csv".replace('.', 'p')
    )

    with open(result_file, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([
            "geo.Z", "geo.X", "geo.H", "geo.W",
            "thrust_avg", "thrust_std", "thrust_max", "thrust_min"
        ])
        writer.writerow([
            geo_Z, geo_X, geo_H, geo_W,
            stats['thrust_avg'], stats['thrust_std'],
            stats['thrust_max'], stats['thrust_min']
        ])

    print(f"[✅] Wrote results to {result_file}")

except Exception as e:
    print(f"[❌] Sample failed for Z={geo_Z}, X={geo_X}, H={geo_H}, W={geo_W}")
    print(f"[ERROR] {e}")

# Save results
result_file = os.path.join(SCRATCH_BASE, "lhs_samples", f"result_Z{geo_Z}_X{geo_X}_H{geo_H}_W{geo_W}.csv".replace('.', 'p'))
with open(result_file, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["geo.Z", "geo.X", "geo.H", "geo.W", "thrust_avg", "thrust_std", "thrust_max", "thrust_min"])
    writer.writerow([
        geo_Z, geo_X, geo_H, geo_W,
        stats['thrust_avg'], stats['thrust_std'],
        stats['thrust_max'], stats['thrust_min']
    ])

print(f"[✅] Wrote results to {result_file}")
